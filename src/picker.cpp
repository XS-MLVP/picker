#include <iostream>
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sys/wait.h>
#include <unordered_set>
#include "picker.hpp"
#include "codegen/lib.hpp"
#include "codegen/mem_direct.hpp"
#include "codegen/cpp.hpp"
#include "codegen/python.hpp"
#include "codegen/java.hpp"
#include "codegen/scala.hpp"
#include "codegen/golang.hpp"
#include "codegen/lua.hpp"
#include "codegen/sv.hpp"
#include "codegen/uvm.hpp"
#include "codegen/firrtl.hpp"
#include "parser/parser_map.hpp"
#include "parser/internalcfg.hpp"
#include "parser/sv.hpp"
#include "parser/uvm.hpp"
#include "parser/verilator_trace.hpp"
#include "parser/native_layout.hpp"
#include "config_database.hpp"
#include "rtl_config.hpp"

picker::main_opts main_opts;
picker::export_opts export_opts;
picker::pack_opts pack_opts;
char *picker::lib_random_hash;
std::map<std::string, std::string> lang_lib_map  = {{"cpp", "lib"},
                                                    {"java", "java/xspcomm-java.jar"},
                                                    {"scala", "scala/xspcomm-scala.jar"},
                                                    {"python", "python"},
                                                    {"golang", "golang"},
                                                    {"lua", "lua/luaxspcomm.so"}};
std::map<std::string, std::string> display_names = {{"cpp", "Cpp"},       {"java", "Java"},     {"scala", "Scala"},
                                                    {"python", "Python"}, {"golang", "Golang"}, {"lua", "Lua"}};

namespace picker {
bool is_debug = false;
}

void check_debug()
{
    if (std::getenv("PICKER_DEBUG") != NULL) { picker::is_debug = true; }
}

std::string to_base62(uint64_t num)
{
    const std::string base62_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string result;
    do {
        result = base62_chars[num % 62] + result;
        num >>= 6;
    } while (num > 0);
    return result;
}

int set_options_export_rtl(CLI::App &top_app)
{
    auto app = top_app.add_subcommand("export", "Export RTL Projects Sources as Software libraries such as C++/Python");

    // Set DUT RTL Source File, conditionally required
    // Note: Required only when --sname/--source_module_name is NOT specified
    app->add_option("file", export_opts.file,
                    "DUT .v/.sv source file. Required if --sname is not specified.")
        ->delimiter(',');

    // Set DUT RTL Extra Source File List, Optional
    app->add_option("--fs,--filelist", export_opts.filelists,
                    "DUT .v/.sv source files, contain the top module, split by comma.\n"
                    "Or use '*.txt' file  with one RTL file path per line to specify the file list")
        ->delimiter(',');

    // Set DUT RTL Simulator, Optional, default is verilator
    app->add_option("--sim", export_opts.sim, "vcs, gsim or verilator as simulator, default is verilator")
        ->default_val("verilator");

    // Set DUT RTL RW Type, Optional, default is DPI
    app->add_option("--rw,--access-mode", export_opts.rw_type,
                    "How to drive the hardware model, DPI or MEM_DIRECT(verilator only), default is DPI")
        ->transform(CLI::CheckedTransformer(
            std::map<std::string, picker::SignalAccessType>{{"dpi", picker::SignalAccessType::DPI},
                                                            {"mem_direct", picker::SignalAccessType::MEM_DIRECT}},
            CLI::ignore_case));

    // Set DUT RTL Language, Optional, default is python
    std::vector<std::string> select_languages = {"python", "cpp", "java", "scala", "golang", "lua"};
    app->add_option("--lang,--language", export_opts.language,
                    "Build target project with assigned language, default python")
        ->default_val("python")
        ->check(CLI::IsMember(select_languages));

    // Set DUT RTL Source Dir, Optional, default is
    // ${picker_install_path}/../picker/template
    app->add_option("--sdir,--source_dir", export_opts.source_dir,
                    "Template Files Dir, default is ${picker_install_path}/../picker/template")
        ->default_val(picker::get_template_path());

    // Set DUT RTL Source Module Name, Optional, default is the last module in
    app->add_option("--sname,--source_module_name", export_opts.source_module_name_list,
                    "Pick the module in DUT .v file, default is the last module in the -f marked file")
        ->delimiter(',');

    // Set Generated Software library module name, Optional, default is the same
    app->add_option(
        "--tname,--target_module_name", export_opts.target_module_name,
        "Set the module name and file name of target DUT, default is the same as source. \nFor example, -T top, will generate UTtop.cpp and UTtop.hpp with UTtop class");

    // Set DUT RTL Target Dir, Optional, default is TNAME
    app->add_option(
           "--tdir,--target_dir", export_opts.target_dir,
           "Target directory to store all the results. If it ends with '/' or is empty, \nthe directory name will be the same as the target module name")
        ->default_val("");

    // Set DUT RTL Internal Signal Config File, Optional, default is empty
    app->add_option("--internal", export_opts.internal,
                    "Exported internal signal config file, default is empty, means no internal pin");

    app->add_flag("--checkpoints", export_opts.checkpoints, "Enable checkpoints, save/restore , default is OFF");
    app->add_flag("--vpi", export_opts.vpi, "Enable VPI, for flexible internal signal access default is OFF");

    // Simulate frequency while using VCS simulator, Optional, default is 100MHz
    app->add_option("-F,--frequency", export_opts.frequency,
                    "Set the frequency of the **only VCS** DUT, default is 100MHz, use Hz, KHz, MHz, GHz as unit")
        ->default_val("100MHz");

    // Dump wave file name, Optional, default is empty means don't dump wave
    app->add_option("-w,--wave_file_name", export_opts.wave_file_name, "Wave file name, empty means don't dump wave");

    // Enable coverage, Optional, default is OFF
    app->add_flag("-c,--coverage", export_opts.coverage, "Enable coverage, default is not selected as OFF");

    // Select Verdi integration mode for VCS simulator, Optional, default is legacy
    app->add_option("--verdi-mode", export_opts.verdi_mode,
                    "Select Verdi integration mode when compiling with VCS.\n"
                    "legacy: use -P novas.tab (default, for older Verdi).\n"
                    "modern: use -debug_access+all (required for Verdi >= 2024.09).\n"
                    "Only applies when --sim vcs.")
        ->default_val("legacy")
        ->check(CLI::IsMember(std::set<std::string>{"legacy", "modern"}, CLI::ignore_case));

    // Enable copy xspcomm lib to DUT location
    app->add_option("--cp_lib, --copy_xspcomm_lib", export_opts.cp_lib,
                    "Copy xspcomm lib to generated DUT dir, default is true")
        ->default_val(true);

    // User defined simulator compile args, passthrough. Eg: '-v -x-assign=fast
    // -Wall --trace' || '-C vcs -cc -f filelist.f'
    app->add_option(
        "-V,--vflag", export_opts.vflag,
        "User defined simulator compile args, passthrough. \nEg: '-v -x-assign=fast -Wall --trace' || '-C vcs -cc -f filelist.f'");

    // User defined slang (RTL frontend) args, passthrough. Eg: '--cmd-ignore=-myflag,1'
    app->add_option("-S,--sflag", export_opts.sflag,
                    "User defined slang RTL parser args, passthrough. \nEg: '--cmd-ignore=-myflag,1'");

    // User defined gcc/clang compile command, passthrough. Eg:'-O3 -std=c++17
    // -I./include'
    app->add_option("-C,--cflag", export_opts.cflag,
                    "User defined gcc/clang compile command, passthrough. Eg:'-O3 -std=c++17 -I./include'");

    // Design parameters are an RTL elaboration input. Picker evaluates them
    // with the RTL frontend and pins the resulting constants in the generated
    // wrapper, so the simulator never needs its own -G.
    app->add_option("--rtl-param", export_opts.rtl_param,
                    "RTL design parameter override, NAME=VALUE. Repeatable.\n"
                    "Only parameters declared in the module parameter port list are accepted.\n"
                    "The value may be an expression; the generated wrapper pins the evaluated constant.\n"
                    "Do not pass -G to -V: the generated wrapper is the simulator's top module.")
        ->type_name("NAME=VALUE");

    // Macros must be visible to both the RTL frontend and the simulator, so
    // picker translates one entry into each tool's syntax.
    app->add_option("--rtl-define", export_opts.rtl_define,
                    "RTL preprocessor macro, NAME or NAME=VALUE. Repeatable.\n"
                    "Applied to the RTL frontend and the simulator, and takes precedence over\n"
                    "the same macro in a filelist. Use this instead of -S '-D...' + -V.")
        ->type_name("NAME[=VALUE]");

    // app->add_option(
    //     "-r,--rename", export_opts.rename,
    //     "User defined gcc/clang compile command, passthrough. Eg:'-O3
    //     -std=c++17 -I./include'");

    app->add_flag("--verbose", export_opts.verbose, "Verbose mode");
    app->add_flag("-e,--example", export_opts.example, "Build example project, default is OFF");
    app->add_option("--autobuild", export_opts.autobuild, "Auto build the generated project, default is true")
        ->default_val(true);
    app->add_option("-j,--build-threads", export_opts.build_threads,
                    "Maximum parallel build threads; 0 uses all available CPU cores")
        ->default_val(0)
        ->check(CLI::NonNegativeNumber);

    return 0;
}

int set_options_pack_message(CLI::App &top_app)
{
    auto app = top_app.add_subcommand("pack", "Pack UVM transaction as a UVM agent and Python class");

    // Set DUT RTL Source File, Required
    app->add_flag("-e,--example", pack_opts.example, "Generate example project based on transaction, default is OFF")
        ->default_val(false);

    app->add_flag("-c,--force", pack_opts.force, "Force delete folder when the code has already generated by picker")
        ->default_val(false);

    app->add_flag("-d,--generate-dut", pack_opts.generate_dut, "Generate DUT abstraction class with pin-level interface (using xspcomm)")
        ->default_val(false);

    app->add_option("--from-rtl", pack_opts.from_rtl_file, "RTL source file (.v/.sv) to auto-generate transaction from module ports");

    app->add_option("file", pack_opts.files, "Sv source file, contain the transaction define");

    app->add_option("-r,--rename", pack_opts.rename, "Rename transaction name in picker generate code")
        ->check([](const std::string &value) {
            // Rejected during parsing, before any file is read or written.
            return picker::codegen::is_valid_package_name(value)
                       ? std::string{}
                       : "'" + value + "' is not a valid name: expected a single identifier matching "
                           "[A-Za-z_][A-Za-z0-9_]*";
        });

    app->add_option("-f,--filelist", pack_opts.filelist, "File list containing transaction files");

    app->add_option("-n,--name", pack_opts.name, "Name for the generated package (default: auto-generated from files)")
        ->check([](const std::string &value) {
            return picker::codegen::is_valid_package_name(value)
                       ? std::string{}
                       : "'" + value + "' is not a valid name: expected a single identifier matching "
                           "[A-Za-z_][A-Za-z0-9_]*";
        });

    return 0;
}

int set_options_main(CLI::App &app)
{
    app.add_flag("-v, --version", main_opts.version, "Print version");
    app.add_flag("--show_default_template_path", main_opts.show_default_template_path, "Print default template path");
    app.add_flag("--show_xcom_lib_location_cpp", main_opts.show_xcom_lib_location_cpp,
                 "Print xspcomm lib and include location");
    app.add_flag("--show_xcom_lib_location_java", main_opts.show_xcom_lib_location_java,
                 "Print xspcomm-java.jar location");
    app.add_flag("--show_xcom_lib_location_scala", main_opts.show_xcom_lib_location_scala,
                 "Print xspcomm-scala.jar location");
    app.add_flag("--show_xcom_lib_location_python", main_opts.show_xcom_lib_location_python,
                 "Print python module xspcomm location");
    app.add_flag("--show_xcom_lib_location_golang", main_opts.show_xcom_lib_location_golang,
                 "Print golang module xspcomm location");
    app.add_flag("--show_xcom_lib_location_lua", main_opts.show_xcom_lib_location_lua,
                 "Print lua module xspcomm location");
    app.add_flag("--check", main_opts.check, "check install location and supported languages");
    return 0;
}

int show_xcom_lib_location()
{
    std::string err_message;

#define SHOW_CPP_LOCATION(flag)                                                                                        \
    if (main_opts.flag) {                                                                                              \
        auto lib_location = picker::get_xcomm_lib("lib", err_message);                                                 \
        if (lib_location.empty()) {                                                                                    \
            PK_ERROR("%s", err_message.c_str());                                                                       \
            exit(1);                                                                                                   \
        }                                                                                                              \
        auto include_location = picker::get_xcomm_lib("include", err_message);                                         \
        if (include_location.empty()) {                                                                                \
            PK_ERROR("%s", err_message.c_str());                                                                       \
            exit(1);                                                                                                   \
        }                                                                                                              \
        PK_MESSAGE("Lib:     %s", lib_location.c_str());                                                               \
        PK_MESSAGE("Include: %s", include_location.c_str());                                                           \
        exit(0);                                                                                                       \
    }

#define SHOW_SINGLE_LOCATION(flag, path)                                                                               \
    if (main_opts.flag) {                                                                                              \
        auto location = picker::get_xcomm_lib(path, err_message);                                                      \
        if (location.empty()) {                                                                                        \
            PK_ERROR("%s", err_message.c_str());                                                                       \
            exit(1);                                                                                                   \
        }                                                                                                              \
        PK_MESSAGE("%s", location.c_str());                                                                            \
        exit(0);                                                                                                       \
    }

    SHOW_CPP_LOCATION(show_xcom_lib_location_cpp);
    SHOW_SINGLE_LOCATION(show_xcom_lib_location_java, "java/xspcomm-java.jar");
    SHOW_SINGLE_LOCATION(show_xcom_lib_location_scala, "scala/xspcomm-scala.jar");
    SHOW_SINGLE_LOCATION(show_xcom_lib_location_python, "python");
    SHOW_SINGLE_LOCATION(show_xcom_lib_location_golang, "golang");
    SHOW_SINGLE_LOCATION(show_xcom_lib_location_lua, "lua/luaxspcomm.so");

    return 0;
}

namespace {

    std::vector<std::string> split_cli_tokens(const std::string &text)
    {
        std::vector<std::string> tokens;
        std::string current;
        for (char c : text) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (!current.empty()) { tokens.push_back(current); }
                current.clear();
            } else {
                current += c;
            }
        }
        if (!current.empty()) { tokens.push_back(current); }
        return tokens;
    }

    // -S and -V are uninterpreted passthroughs. Two settings look like they
    // work there but cannot: a simulator -G never reaches the instantiated
    // design, and a macro given to only one frontend desynchronizes the port
    // widths. Point at the structured options instead of failing silently.
    void warn_about_raw_overrides(const picker::export_opts &opts)
    {
        bool warned_param = false, warned_define = false, warned_frontend_define = false;
        for (const auto &token : split_cli_tokens(opts.vflag)) {
            if (!warned_param && (token.starts_with("-G") || token.starts_with("-pvalue+"))) {
                warned_param = true;
                PK_OUTPUT(stderr, "%s", "\033[33m");
                PK_OUTPUT(stderr,
                          "[Warning] -V contains '%s'. A simulator parameter override cannot reach the "
                          "instantiated design; use --rtl-param NAME=VALUE.\n",
                          token.c_str());
                PK_OUTPUT(stderr, "%s", "\033[0m");
            } else if (!warned_define && (token.starts_with("+define+") || token.starts_with("-D"))) {
                warned_define = true;
                PK_OUTPUT(stderr, "%s", "\033[33m");
                PK_OUTPUT(stderr,
                          "[Warning] -V contains '%s'. The RTL frontend will not see this macro; "
                          "use --rtl-define NAME[=VALUE] to define it for both.\n",
                          token.c_str());
                PK_OUTPUT(stderr, "%s", "\033[0m");
            }
        }
        for (const auto &token : opts.sflag) {
            if (!warned_frontend_define && (token.starts_with("-D") || token.starts_with("+define+"))) {
                warned_frontend_define = true;
                PK_OUTPUT(stderr, "%s", "\033[33m");
                PK_OUTPUT(stderr,
                          "[Warning] -S contains '%s'. The simulator will not see this macro; "
                          "use --rtl-define NAME[=VALUE] to define it for both.\n",
                          token.c_str());
                PK_OUTPUT(stderr, "%s", "\033[0m");
            }
        }
    }

    // --rtl-define must reach the simulator as well as the RTL frontend. The
    // simulator side is a compile flag, so it is appended to the passthrough
    // string that the templates already carry. Parameters intentionally do not
    // follow this path: they are pinned in the generated wrapper instead.
    void apply_rtl_defines_to_vflag(picker::export_opts &opts)
    {
        const auto defines = picker::rtl::parse_defines(opts.rtl_define, "--rtl-define");
        if (defines.empty()) { return; }
        std::string extra;
        for (const auto &define : defines) {
            if (!extra.empty()) { extra += " "; }
            // The flag string is written into the generated Makefile and later
            // split by CMake's separate_arguments(NATIVE_COMMAND). Escape the
            // apostrophes of SV literals (1'b0) so that splitter does not
            // consume them as quoting.
            extra += picker::rtl::escape_vflag_apostrophes(picker::rtl::simulator_define_argument(define));
        }
        if (!opts.vflag.empty()) { opts.vflag += " "; }
        opts.vflag += extra;
    }

    // Module names become RTL identifiers, file names and Makefile variables.
    // `$` is legal in a SystemVerilog identifier but starts a Makefile
    // expansion, so `UT_foo$bar` would silently read a variable named `b` and
    // produce `UT_fooar`. Accept only the subset that is an identifier in all
    // three places.
    bool is_sv_module_name(const std::string &name)
    {
        if (name.empty()) { return false; }
        const char first = name.front();
        if (!((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z') || first == '_')) {
            return false;
        }
        for (char c : name) {
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                c == '_') { continue; }
            return false;
        }
        return true;
    }

    // --sname also accepts "<module>,<count>" entries. Counts are not module
    // names, but every other entry must be a legal identifier; otherwise it
    // could escape the target directory or inject Makefile syntax.
    void validate_module_names()
    {
        // --sname is a flat list: ModuleA,2,ModuleB. A count only means
        // something directly after a module name; accepting a leading or
        // repeated count would silently fall back to "all modules".
        bool expects_count = false;
        for (const auto &raw : export_opts.source_module_name_list) {
            const auto token = picker::trim(raw);
            if (token.empty()) { continue; } // parse_mname_and_numbers reports empty entries itself
            if (std::isdigit(static_cast<unsigned char>(token.front()))) {
                const bool count = std::all_of(token.begin(), token.end(), [](char c) {
                    return std::isdigit(static_cast<unsigned char>(c));
                });
                if (!count) {
                    PK_FATAL("--sname '%s': entries that start with a digit must be instance counts",
                             raw.c_str());
                }
                if (!expects_count) {
                    PK_FATAL("--sname '%s': an instance count must directly follow a module name",
                             raw.c_str());
                }
                int value = 0;
                const char *begin = token.data();
                const char *end = begin + token.size();
                const auto parsed = std::from_chars(begin, end, value);
                if (parsed.ec != std::errc() || parsed.ptr != end || value < 1) {
                    PK_FATAL("--sname '%s': instance count must be an integer between 1 and %d",
                             raw.c_str(), std::numeric_limits<int>::max());
                }
                expects_count = false;
                continue;
            }
            if (!is_sv_module_name(token)) {
                PK_FATAL("--sname '%s': '%s' is not a legal module name for a generated "
                         "wrapper ([A-Za-z_][A-Za-z0-9_]*; `$` is rejected because it expands "
                         "in the generated Makefile)", raw.c_str(), token.c_str());
            }
            expects_count = true;
        }
        if (!export_opts.target_module_name.empty() && !is_sv_module_name(export_opts.target_module_name)) {
            PK_FATAL("--tname '%s' is not a legal module name for a generated wrapper "
                     "([A-Za-z_][A-Za-z0-9_]*; `$` is rejected because it expands in the "
                     "generated Makefile)", export_opts.target_module_name.c_str());
        }
    }

    // Verilator treats a differing macro redefinition as a fatal REDEFMACRO
    // warning, so a filelist macro overridden by --rtl-define must be removed
    // from the generated filelist rather than repeated after it.
    void filter_generated_filelist(const std::string &path,
                                   const std::unordered_set<std::string> &overridden)
    {
        if (overridden.empty()) { return; }
        std::ifstream input(path);
        if (!input.is_open()) { return; }
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(input, line)) {
            auto text = picker::trim(line);
            if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
                text = text.substr(1, text.size() - 2);
            }
            std::string name, replacement;
            std::vector<std::string> argv;
            if (text.starts_with("+define+")) {
                name = "+define";
                argv = {text};
            } else if (text.starts_with("-D")) {
                const auto space = text.find(' ');
                if (space == std::string::npos) {
                    name = text.substr(0, text.find('='));
                    argv = {text};
                } else {
                    name = text.substr(0, space);
                    argv = {name, picker::trim(text.substr(space + 1))};
                }
            } else {
                lines.push_back(line);
                continue;
            }
            if (!picker::rtl::filter_filelist_define(name, argv, overridden, replacement)) { continue; }
            lines.push_back(replacement.empty() ? line : replacement);
        }
        input.close();

        std::ofstream output(path, std::ios::trunc);
        for (const auto &kept : lines) { output << kept << "\n"; }
    }

    // Run make in the export directory without a shell: target_dir is user
    // input, so shell interpolation would both break on spaces and execute
    // metacharacters. fork/execvp keeps the argument a plain path.
    int run_make(const std::string &directory)
    {
        std::fflush(nullptr);
        const pid_t pid = fork();
        if (pid < 0) { PK_FATAL("Failed to fork make: %s", std::strerror(errno)); }
        if (pid == 0) {
            if (chdir(directory.c_str()) != 0) {
                PK_OUTPUT(stderr, "Failed to enter target directory '%s': %s\n", directory.c_str(),
                          std::strerror(errno));
                _exit(127);
            }
            // execvp searches PATH, matching the previous shell lookup.
            char *const argv[] = {const_cast<char *>("make"), nullptr};
            execvp(argv[0], argv);
            PK_OUTPUT(stderr, "Failed to execute make: %s\n", std::strerror(errno));
            _exit(127);
        }
        int status = 0;
        while (waitpid(pid, &status, 0) < 0) {
            if (errno != EINTR) { PK_FATAL("Failed to wait for make: %s", std::strerror(errno)); }
        }
        if (WIFEXITED(status)) { return WEXITSTATUS(status); }
        if (WIFSIGNALED(status)) { PK_MESSAGE("Build terminated by signal %d", WTERMSIG(status)); }
        return 1;
    }

} // namespace

namespace {

    /// Pack names become the output directory and generated identifiers, so a
    /// single [A-Za-z_][A-Za-z0-9_]* token is the only accepted form. Validated
    /// at argument parsing and again before any filesystem change; the last gate
    /// is remove_pack_output() in src/codegen/uvm.cpp.
    void require_pack_name(const std::string &name, const char *option)
    {
        if (!picker::codegen::is_valid_package_name(name)) {
            PK_FATAL("%s '%s' is not a valid name: expected a single identifier matching "
                     "[A-Za-z_][A-Za-z0-9_]*",
                     option, name.c_str());
        }
    }

} // namespace

int check_picker_support()
{
    std::string err_message;

    // Validate the structured RTL configuration before any expensive work, so a
    // typo fails immediately instead of after elaboration.
    picker::rtl::parse_params(export_opts.rtl_param, "--rtl-param");
    picker::rtl::parse_defines(export_opts.rtl_define, "--rtl-define");
    validate_module_names();
    if (export_opts.sim == "gsim" && (!export_opts.rtl_param.empty() || !export_opts.rtl_define.empty())) {
        PK_FATAL("--rtl-param and --rtl-define require a SystemVerilog frontend (verilator, vcs, uvs); "
                 "--sim gsim consumes FIRRTL input.");
    }
    warn_about_raw_overrides(export_opts);

    // check if the xspcomm lib is available
    if (main_opts.check) {
        PK_MESSAGE("[OK ] Version: %s-%s-%s%s", PROJECT_VERSION, GIT_BRANCH, GIT_HASH, GIT_DIRTY);
        PK_MESSAGE("[OK ] Exec path: %s", picker::get_executable_path().c_str());
        auto temp_path = picker::get_template_path();
        if (temp_path.empty()) {
            PK_MESSAGE("[Err] Can't find default template path");
        } else {
            PK_MESSAGE("[OK ] Template path: %s", temp_path.c_str());
        }
        for (auto &kv : lang_lib_map) {
            auto lib_location = picker::get_xcomm_lib(kv.second, err_message);
            if (lib_location.empty()) {
                PK_ERROR("[Err] Support %6s (find: '%s' fail)", display_names[kv.first].c_str(), kv.second.c_str());
            } else {
                PK_MESSAGE("[OK ] Support %6s (find: '%s' success)", display_names[kv.first].c_str(),
                           lib_location.c_str());
            }
        }
        exit(0);
    }

    // check if the language is supported
    if (!export_opts.language.empty()) {
        auto it = lang_lib_map.find(export_opts.language);
        if (it == lang_lib_map.end()) {
            PK_ERROR("[Err] Unknown language '%s'", export_opts.language.c_str());
            exit(1);
        }
        auto lib_location = picker::get_xcomm_lib(it->second, err_message);
        if (lib_location.empty()) {
            PK_ERROR("[Err] Support %6s (find: '%s' fail)", display_names[it->first].c_str(), it->second.c_str());
            exit(1);
        }
    }

    // check if the rw_type is supported
    switch (export_opts.rw_type) {
    case picker::SignalAccessType::DPI:
        if (export_opts.internal != "") { PK_MESSAGE("It's recommended to use MEM_DIRECT for internal signal access"); }
        break;
    case picker::SignalAccessType::MEM_DIRECT:
        std::unordered_set<std::string> supported_simulators = {"verilator", "gsim"};
        if (supported_simulators.count(export_opts.sim) == 0) {
            PK_ERROR("[Err] MEM_DIRECT not support simulator '%s'", export_opts.sim.c_str());
        }
        break;
    }

    return 0;
}


int main(int argc, char **argv)
{
    check_debug();
    const bool inspecting_config = argc > 1 && std::string_view(argv[1]) == "config";
    if(picker::appimage::is_running_as_appimage()) {
        if (!inspecting_config) PK_MESSAGE("Running as AppImage");
        picker::appimage::initialize();
    } else if (!inspecting_config) {
        PK_MESSAGE("Running as normal executable");
    }
    CLI::App app{"XDut Generate. \n"
                 "Convert DUT(*.v/*.sv) to C++ DUT libs.\n"};
    set_options_main(app);
    set_options_export_rtl(app);
    set_options_pack_message(app);
    std::string config_path, config_section, config_name;
    auto config_app = app.add_subcommand("config", "Inspect a single-file XSCFG database (JSON to stdout)");
    config_app->add_option("file", config_path)->required();
    config_app->add_option("--section", config_section, "metadata, variables, signals, types or candidates");
    config_app->add_option("--name", config_name, "Inspect one record without loading the whole database")->needs("--section");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        // PK_MESSAGE("%s", app.help().c_str());
        return app.exit(e);
    }

    if (config_app->parsed()) {
        try {
            picker::config::db::Database input(config_path);
            nlohmann::json out;
            if (config_section.empty()) out = {{"format_version", picker::config::db::format_version},
                {"model_id", input.model()}, {"bytes", input.size()}, {"records", input.records()}};
            else {
                const std::map<std::string, char> sections{{"metadata", 'm'}, {"variables", 'v'},
                    {"signals", 's'}, {"types", 't'}, {"candidates", 'c'}};
                auto section = sections.find(config_section);
                if (section == sections.end()) throw std::invalid_argument("Unknown configuration section");
                if (!config_name.empty()) {
                    auto value = input.get(section->second, config_name);
                    if (!value) throw std::invalid_argument("Configuration record not found: " + config_name);
                    out = picker::config::decode(*value);
                } else {
                    out = nlohmann::json::object();
                    input.each(section->second, [&](const auto &name, const auto &v) { out[name] = picker::config::decode(v); });
                }
            }
            std::cout << out.dump(2) << '\n'; return 0;
        } catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
    }

    // handle main options here
    // if need version, print version
    if (main_opts.version) {
        PK_MESSAGE("version: %s-%s-%s%s", PROJECT_VERSION, GIT_BRANCH, GIT_HASH, GIT_DIRTY);
        exit(0);
    }

    // get random for random library name
    srand48(time(NULL));
    std::string hash_name   = to_base62(mrand48() << 32 | mrand48());
    picker::lib_random_hash = new char[hash_name.size() + 1];
    std::copy(hash_name.begin(), hash_name.end(), picker::lib_random_hash);
    picker::lib_random_hash[hash_name.size()] = '\0';

    if (main_opts.show_default_template_path) {
        auto temp_path = picker::get_template_path();
        if (temp_path.empty()) { PK_FATAL("Can't find default template path"); }
        PK_MESSAGE("%s", temp_path.c_str());
        exit(0);
    }

    show_xcom_lib_location();
    check_picker_support();

    // check if app parsed export subcommand
    if (app.get_subcommands().size() == 0) {
        PK_MESSAGE("%s", app.help().c_str());
        PK_MESSAGE("No subcommand specified: need export or pack");
        exit(0);
    }

    // subcommand export
    if (app.get_subcommand_ptr("export")->parsed()) {

        // render mem_direct depends on the verilator codegen, so we need to render it between verilator and dut_base
        if(export_opts.source_dir.ends_with("/mem_direct")){
            PK_MESSAGE("Rendering mem_direct");         
            // picker is invoked by codegen Makefile, so the export_opts.file is NOT sv but the Verilator CPP (for now).
            std::string source_file = export_opts.file[0];
            std::vector<std::string> declarations;
            std::vector<picker::cpp_variableInfo> vars;

            auto readVarDeclarations = picker::parser::GetReadVarDeclarations(export_opts.sim);
            auto processDeclarations = picker::parser::GetProcessDeclarations(export_opts.sim);
            declarations = readVarDeclarations(source_file);
            vars = processDeclarations(declarations);

            const auto trace_files = picker::parser::verilator::findTraceFiles(source_file);
            picker::parser::trace_parse_result trace_result;
            if (trace_files.empty()) {
                PK_MESSAGE("Trace files not found near %s, skipping trace-derived signal map", source_file.c_str());
            } else {
                PK_MESSAGE("Parsing %zu trace file(s)", trace_files.size());
                trace_result = picker::parser::verilator::processTraceFiles(source_file, trace_files, vars);
                if (trace_result.signals.empty()) {
                    PK_FATAL("Trace files were found, but no usable signal mappings could be built "
                             "(decl=%zu, value=%zu, matched_slots=%zu). "
                             "This likely means trace was generated in an unsupported Verilator format/version.",
                             trace_result.decl_count, trace_result.value_count, trace_result.matched_slot_count);
                }
            }

            // Semantic metadata belongs to this generated project, not to the
            // host installation. Match only retained native variables: RTL
            // declarations alone are not evidence of addressable storage.
            const auto layout_file = std::filesystem::path(export_opts.target_dir).parent_path() /
                                     (export_opts.source_module_name_list[0] + ".xscfg");
            auto native_types = nlohmann::json::object();
            if (std::filesystem::exists(layout_file)) {
                picker::config::db::Database input(layout_file.string());
                auto layouts = nlohmann::json{{"version", 1}, {"types", nlohmann::json::object()}, {"signals", nlohmann::json::object()}};
                input.each('t', [&](const auto &id, const auto &v) { layouts["types"][id] = picker::config::decode(v); });
                input.each('c', [&](const auto &name, const auto &v) { layouts["signals"][name] = picker::config::decode(v); });
                if (export_opts.sim == "verilator")
                    native_types = picker::parser::bind_native_layouts(source_file, layouts, vars, trace_result.signals);
                picker::config::db::Writer updated(input.model());
                updated.copy(input, "mct");
                for (const auto &signal : trace_result.signals) {
                    auto node = nlohmann::json{{"kind", signal.kind}, {"type", signal.type},
                        {"rtl_width", signal.width}, {"source", signal.root_name},
                        {"expr", signal.source_expr}, {"value", signal.source_expr}, {"deps", nlohmann::json::array()}};
                    for (const auto &dep : signal.deps) node["deps"].push_back(dep.name);
                    updated.add('q', signal.name, picker::config::encode(node));
                }
                updated.write(layout_file.string());
            }

            picker::codegen::render_md_addr_generator(vars, export_opts, native_types);
            exit(0);
        }

        std::vector<picker::sv_module_define> sv_module_result; // sv signal pins
        std::vector<picker::sv_signal_define> internal_signal_result; // configuration signal pins

        nlohmann::json signal_tree_json;
        auto input_parser = picker::parser::GetInputParser(export_opts.sim);
        input_parser(export_opts, sv_module_result);
        picker::parser::internal(export_opts, internal_signal_result);

        // Macros must reach the simulator too. The frontend side was already
        // applied while preparing the RTL parse.
        apply_rtl_defines_to_vflag(export_opts);
        auto sv_pin_result =
            picker::codegen::lib(export_opts, sv_module_result, internal_signal_result, signal_tree_json);

        // The frontend read the original filelists; only the generated one is
        // handed to the simulator, so overridden macro definitions are dropped
        // here instead of being repeated after the command-line value.
        std::unordered_set<std::string> overridden_defines;
        for (const auto &define : picker::rtl::parse_defines(export_opts.rtl_define, "--rtl-define")) {
            overridden_defines.insert(define.name);
        }
        filter_generated_filelist(export_opts.target_dir + "/filelist.f", overridden_defines);

        std::map<std::string, std::function<void(picker::export_opts &, std::vector<picker::sv_signal_define>,
                                                 std::vector<picker::sv_signal_define>, nlohmann::json &)>>
            func_map = {
                {"cpp", picker::codegen::cpp},     {"python", picker::codegen::python}, {"java", picker::codegen::java},
                {"scala", picker::codegen::scala}, {"golang", picker::codegen::golang}, {"lua", picker::codegen::lua},
            };
        func_map[export_opts.language](export_opts, sv_pin_result, internal_signal_result, signal_tree_json);

        // build the result with make
        if (export_opts.autobuild) {
            if (run_make(export_opts.target_dir) != 0) {
                PK_MESSAGE("Build failed");
                exit(1);
            }
        }
        // subcommand pack
    } else if (app.get_subcommand_ptr("pack")->parsed()) {

        // Validate every user-supplied name before any filesystem change. The
        // CLI checks already reject bad -n/-r values while parsing; repeating
        // the check here also covers empty vector elements, which CLI11 skips.
        if (!pack_opts.name.empty()) { require_pack_name(pack_opts.name, "--name"); }
        for (const auto &rename : pack_opts.rename) { require_pack_name(rename, "--rename"); }

        std::vector<picker::uvm_transaction_define> transactions;
        std::vector<std::string> filenames;

        if (!pack_opts.from_rtl_file.empty()) {
            // RTL Mode: auto-generate transaction from module ports
            PK_MESSAGE("Mode: RTL (auto-generate transaction from module ports)");

            std::string module_name;
            transactions.push_back(picker::parser::parse_rtl_file(pack_opts.from_rtl_file, module_name, pack_opts.name));
            filenames.push_back(module_name);

        } else {
            // Transaction Mode: parse SV transaction classes
            picker::parser::parse_sv_transactions(pack_opts, transactions);

            PK_MESSAGE("Mode: %s Transaction%s", transactions.size() == 1 ? "Single" : "Multi",
                       transactions.size() > 1 ? ("s (" + std::to_string(transactions.size()) + " files)").c_str() : "");

            for (size_t i = 0; i < transactions.size(); i++) {
                auto &transaction = transactions[i];
                std::string filename = (i < pack_opts.rename.size()) ? pack_opts.rename[i]
                                      : std::filesystem::path(transaction.filepath).stem().string();
                if (i < pack_opts.rename.size()) {
                    transaction.name = pack_opts.rename[i];
                }

                filenames.push_back(filename);
            }
        }

        std::string package_name = !pack_opts.name.empty() ? pack_opts.name :
                                  (!filenames.empty() ? filenames[0] : "");

        if (package_name.empty()) {
            PK_FATAL("Cannot determine package name");
        }

        // Without -n the name is derived from the first input file stem, which
        // may itself be path-like (e.g. the file was given as "../.."). Check it
        // before it becomes the directory that --force can delete.
        require_pack_name(package_name, "package name");

        PK_MESSAGE("Package name: %s", package_name.c_str());

        auto package_data = picker::parser::prepare_uvm_package_data(
            transactions,
            filenames,
            package_name,
            pack_opts.generate_dut,
            pack_opts.from_rtl_file
        );

        // Generate UVM package files
        picker::codegen::generate_uvm_package(package_data, pack_opts, package_name);

        PK_MESSAGE("Successfully generated UVM package: %s", package_name.c_str());
        exit(0);
    }

    nlohmann::json sync_opts;
    return 0;
}
