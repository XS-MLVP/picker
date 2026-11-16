#include "codegen/uvm.hpp"
#include "codegen/pack_path_guard.hpp"
#include "parser/sv.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace picker { namespace codegen {

    // ============================================================================
    // Template Rendering
    // ============================================================================

    /// @brief Render template file with data and write to output
    /// @param data Template data (JSON)
    /// @param template_path Path to template file
    /// @param output_path Path to output file
    static void render_template(const inja::json& data, const std::string& template_path, const std::string& output_path)
    {
        inja::Environment env;
        env.write(template_path, data, output_path);
    }

    // ============================================================================
    // Directory Setup
    // ============================================================================

    /// @brief Output directory structure for UVM package generation
    struct OutputDirs {
        std::filesystem::path package;  ///< Package directory (contains Python/SV files)
        std::filesystem::path top;      ///< Top directory (contains example files)
        std::filesystem::path build;    ///< Build directory (for compilation artifacts)
    };

    /// @brief Setup output directory structure
    /// @param opts Pack options
    /// @param package_name Package name
    /// @return Output directory structure
    static OutputDirs setup_output_directories(
        const pack_opts& opts,
        const std::string& package_name)
    {
        namespace fs = std::filesystem;

        OutputDirs dirs;

        // Defense 1: the name is used as a directory path and as a class/module
        // identifier. Callers must already validate it, but never let a
        // path-like value ("..", "/tmp", "a/b") reach the filesystem from here.
        if (!is_valid_package_name(package_name)) {
            PK_FATAL("Invalid pack name '%s': expected a single identifier matching [A-Za-z_][A-Za-z0-9_]*",
                     package_name.c_str());
        }

        // Root the deletion is confined to. Everything below this is picker's
        // own output and disposable; anything else is user data.
        fs::path allowed_root;
        if (opts.example) {
            // Example mode: create uvmpy/package_name/build structure
            dirs.top = "uvmpy";
            dirs.package = dirs.top / package_name;
            allowed_root = dirs.top;
        } else {
            // Normal mode: create package_name/build structure
            dirs.top = package_name;
            dirs.package = package_name;
            allowed_root = ".";
        }
        dirs.build = dirs.package / "build";

        std::error_code ec;
        const bool package_exists = fs::exists(dirs.package, ec);
        ec.clear();
        const bool package_is_link = fs::is_symlink(dirs.package, ec);

        // Check if folder exists
        if (package_exists && !opts.force) {
            PK_MESSAGE("folder already exists, use -c/--force to overwrite");
            exit(0);
        }

        // Remove and recreate: defenses 2 and 3 live in remove_pack_output().
        // A dangling symlink counts as "exists" here so it is refused with a
        // diagnostic instead of failing later in create_directories().
        if (package_exists || package_is_link) {
            const std::string refusal = remove_pack_output(dirs.package, allowed_root);
            if (!refusal.empty()) {
                PK_FATAL("Refusing to delete %s. Remove it manually if that is intended.", refusal.c_str());
            }
        }

        ec.clear();
        fs::create_directories(dirs.build, ec);
        if (ec) { PK_FATAL("Cannot create '%s': %s", dirs.build.string().c_str(), ec.message().c_str()); }

        // Mark the tree as a picker product so a later --force run is allowed to
        // delete it (see is_picker_pack_output). Written before the templates are
        // rendered so an interrupted generation can still be cleaned up.
        std::ofstream marker(dirs.package / pack_marker_name, std::ios::trunc);
        if (!marker) {
            PK_FATAL("Cannot write pack marker '%s'", (dirs.package / pack_marker_name).string().c_str());
        }
        marker << "picker pack output\n";
        marker << "package: " << package_name << "\n";
        marker << "version: " << picker::version() << "\n";
        if (!marker) {
            PK_FATAL("Cannot write pack marker '%s'", (dirs.package / pack_marker_name).string().c_str());
        }

        return dirs;
    }

    // ============================================================================
    // File Generation (Pure Template Rendering)
    // ============================================================================

    /// @brief Get UVM template directory path
    /// @return Path to UVM template directory
    static std::filesystem::path get_uvm_template_dir()
    {
        return std::filesystem::path(picker::get_template_path()) / "uvm";
    }

    /// @brief Generate core package files from template data
    /// @param data Complete template data (prepared by parser)
    /// @param pkg_folder Package folder path
    static void generate_core_files(const inja::json& data, const std::filesystem::path& pkg_folder)
    {
        namespace fs = std::filesystem;
        fs::path template_dir = get_uvm_template_dir();

        // Generate agent files (Python and SystemVerilog)
        render_template(data, (template_dir / "xagent.py").string(), (pkg_folder / "xagent.py").string());
        render_template(data, (template_dir / "xagent.sv").string(), (pkg_folder / "xagent.sv").string());

        // Generate package init and utils
        render_template(data, (template_dir / "dut.py").string(), (pkg_folder / "__init__.py").string());
        render_template(data, (template_dir / "utils_pkg.sv").string(), (pkg_folder / "utils_pkg.sv").string());

        // Generate transaction class for RTL-generated transactions
        // Check if this is single-transaction RTL mode
        if (data.contains("transactions") && data["transactions"].size() == 1) {
            const auto& trans = data["transactions"][0];
            if (trans.contains("from_rtl") && trans["from_rtl"].get<bool>()) {
                std::string module_name = trans["name"];
                fs::path trans_sv_path = pkg_folder / (module_name + "_trans.sv");

                // Prepare transaction class data
                inja::json trans_class_data = data;
                trans_class_data["class_name"] = module_name + "_trans";

                render_template(trans_class_data, (template_dir / "transaction_class.sv").string(), trans_sv_path.string());
                PK_MESSAGE("Generated transaction class: %s", trans_sv_path.string().c_str());
            }
        }
    }

    /// @brief Generate example files from template data
    /// @param data Complete template data
    /// @param top_folder Top folder path
    /// @param generate_dut Whether to generate DUT example
    static void generate_example_files(const inja::json& data, const std::filesystem::path& top_folder, bool generate_dut)
    {
        namespace fs = std::filesystem;
        fs::path template_dir = get_uvm_template_dir();

        // Python example (unified template)
        render_template(data, (template_dir / "example.py").string(), (top_folder / "example.py").string());

        // SystemVerilog example and Makefile
        render_template(data, (template_dir / "example.sv").string(), (top_folder / "example.sv").string());
        render_template(data, (template_dir / "Makefile").string(), (top_folder / "Makefile").string());
    }

    // ============================================================================
    // Public API - Pure Rendering Layer
    // ============================================================================

    /// @brief Generate UVM package files from prepared template data
    /// @param data Complete template data (from parser::prepare_uvm_package_data)
    /// @param opts Pack options (for directory setup and example generation)
    /// @param package_name Package name
    void generate_uvm_package(const inja::json& data, const pack_opts& opts, const std::string& package_name)
    {
        // 1. Setup output directories
        OutputDirs dirs = setup_output_directories(opts, package_name);

        // 2. Generate core files
        generate_core_files(data, dirs.package);

        // 3. Generate example files (if requested)
        if (opts.example) {
            generate_example_files(data, dirs.top, opts.generate_dut);
        }

        std::cout << "generate " + package_name + " code successfully." << std::endl;
    }
}} // namespace picker::codegen
