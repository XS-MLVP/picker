#pragma once

// Guardrails for `picker pack` output directories.
//
// The pack name comes from -n/--name, -r/--rename or the first input file stem,
// and is then used directly as a directory path that --force may recursively
// delete (src/codegen/uvm.cpp). These helpers are the shared, dependency-free
// gate, kept header-only so they can be exercised outside the picker binary:
//
//   1. is_valid_package_name - only [A-Za-z_][A-Za-z0-9_]* may name a package,
//      so ".", "..", "/tmp" or "a/b" never reach the filesystem as a path.
//      (The name is also a Python/SystemVerilog identifier in the templates, so
//      a path-like value would produce broken code anyway.)
//   2. is_strictly_within    - the deletion target must resolve strictly below
//      the intended output root, after weakly_canonical() normalization. Paths
//      are compared component by component, so "out2" is not inside "out", and
//      a trailing "/", "." or ".." cannot fake containment.
//   3. is_picker_pack_output - only directories carrying the .picker_pack marker
//      (or the legacy xagent.py/xagent.sv files) may be deleted.

#include <filesystem>
#include <string>
#include <system_error>

namespace picker { namespace codegen {

    /// Name of the marker file written into every generated package.
    inline constexpr const char pack_marker_name[] = ".picker_pack";

    /// @brief True when `name` is a single identifier: [A-Za-z_][A-Za-z0-9_]*
    /// ASCII-only on purpose: the result is used as a directory name, a shell
    /// make variable and a Python/SystemVerilog identifier.
    inline bool is_valid_package_name(const std::string &name)
    {
        if (name.empty()) { return false; }
        const auto is_letter = [](char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); };
        const auto is_digit  = [](char c) { return c >= '0' && c <= '9'; };
        if (!(is_letter(name.front()) || name.front() == '_')) { return false; }
        for (const char c : name) {
            if (!(is_letter(c) || is_digit(c) || c == '_')) { return false; }
        }
        return true;
    }

    /// @brief True when `target` is strictly inside `root`.
    /// Both sides are weakly canonicalized first, which normalizes "." / "..",
    /// drops a trailing separator and resolves symlinks in the existing prefix.
    /// The root itself is not "strictly within", so the boundary is never
    /// deleted.
    inline bool is_strictly_within(const std::filesystem::path &target, const std::filesystem::path &root)
    {
        namespace fs = std::filesystem;
        std::error_code ec;
        const fs::path canonical_target = fs::weakly_canonical(target, ec);
        if (ec) { return false; }
        ec.clear();
        const fs::path canonical_root = fs::weakly_canonical(root, ec);
        if (ec) { return false; }

        auto target_it = canonical_target.begin();
        for (auto root_it = canonical_root.begin(); root_it != canonical_root.end(); ++root_it, ++target_it) {
            if (target_it == canonical_target.end() || *target_it != *root_it) { return false; }
        }
        return target_it != canonical_target.end();
    }

    /// @brief True when `package_dir` carries proof that picker pack created it.
    inline bool is_picker_pack_output(const std::filesystem::path &package_dir)
    {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (!fs::is_directory(package_dir, ec) || ec) { return false; }
        ec.clear();
        // remove_all() deletes the link, not its target, but a symlink is not a
        // tree picker generated, so leave it for the user to inspect.
        if (fs::is_symlink(package_dir, ec) || ec) { return false; }
        ec.clear();
        if (fs::is_regular_file(package_dir / pack_marker_name, ec) && !ec) { return true; }
        // Older packs predate the marker but always shipped both files. Require
        // the pair instead of either one: a stray xagent.py in an unrelated
        // directory must not make that directory deletable.
        return fs::is_regular_file(package_dir / "xagent.py", ec) && !ec &&
               fs::is_regular_file(package_dir / "xagent.sv", ec) && !ec;
    }

    /// @brief Check, then delete an existing pack output directory.
    /// @return Empty string on success, otherwise the reason nothing was deleted.
    inline std::string remove_pack_output(const std::filesystem::path &package_dir,
                                          const std::filesystem::path &allowed_root)
    {
        namespace fs = std::filesystem;
        if (!is_strictly_within(package_dir, allowed_root)) {
            std::error_code ec;
            const auto canonical_root = fs::weakly_canonical(allowed_root, ec);
            const std::string root_text = ec ? allowed_root.string() : canonical_root.string();
            return "'" + package_dir.string() + "': it is not strictly inside the pack output root '" + root_text +
                   "'";
        }
        if (!is_picker_pack_output(package_dir)) {
            return "'" + package_dir.string() + "': it does not look like a picker pack output (no " +
                   std::string(pack_marker_name) + ", xagent.py or xagent.sv)";
        }
        std::error_code ec;
        fs::remove_all(package_dir, ec);
        if (ec) { return "failed to delete '" + package_dir.string() + "': " + ec.message(); }
        return {};
    }

}} // namespace picker::codegen
