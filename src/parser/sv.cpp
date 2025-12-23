#include "picker.hpp"
#include "parser/sv.hpp"
#include "parser/exprtk.hpp"

#include <unordered_set>

#define token_res(i)                                                                                                   \
    (std::string(module_token[i]["tag"])).size() > 1 ? (std::string)module_token[i]["text"] :                          \
                                                       (std::string)module_token[i]["tag"]

#define param_res(var) parameter_var.count(var) ? parameter_var[var] : var

namespace picker { namespace parser {

    // Helper: check if a path is a Verilog/SystemVerilog source file
    static inline bool is_verilog_src(const std::string &p){
        return p.ends_with(".sv") || p.ends_with(".v");
    }

    // Helper: trim comments and whitespace from a line taken from filelist
    static inline std::string strip_comment_and_trim(const std::string &line){
        auto s = picker::trim(line);
        if (s.starts_with("#")) return std::string("");
        auto pos = s.find('#');
        if (pos != std::string::npos) s = picker::trim(s.substr(0, pos));
        return s;
    }

    // Collect .v/.sv files from --fs/--filelist options for module search
    void collect_verilog_from_filelists(const std::vector<std::string> &filelists,
                                               std::vector<std::string> &out)
    {
        namespace fs = std::filesystem;
        std::vector<std::string> entries;
        std::string last_list_base;

        for (const auto &fl : filelists) {
            if (fl.ends_with(".txt") || fl.ends_with(".f")) {
                // Lines inside are relative to the filelist file
                last_list_base = fs::absolute(fl).parent_path().string();
                std::ifstream ifs(fl);
                std::string line;
                while (std::getline(ifs, line)) {
                    entries.push_back(line);
                }
            } else {
                // Comma-separated entries directly from CLI
                std::stringstream ss(fl);
                std::string token;
                while (std::getline(ss, token, ',')) {
                    entries.push_back(token);
                }
            }
        }

        for (auto entry : entries) {
            entry = strip_comment_and_trim(entry);
            if (entry.empty()) continue;

            // Resolve relative path against the last seen filelist base if present
            std::string path = entry;
            if (!fs::exists(path) && !path.starts_with("/") && !last_list_base.empty()) {
                path = (fs::path(last_list_base) / path).string();
            }

            if (!fs::exists(path)) {
                // Not fatal here; filelist may contain non-Verilog or generated paths not yet present
                continue;
            }

            if (fs::is_directory(path)) {
                for (auto const &dir_entry : fs::recursive_directory_iterator(path)) {
                    if (!dir_entry.is_regular_file()) continue;
                    auto p = dir_entry.path().string();
                    if (is_verilog_src(p)) {
                        out.push_back(fs::absolute(p).string());
                    }
                }
            } else if (fs::is_regular_file(path)) {
                if (is_verilog_src(path)) {
                    out.push_back(fs::absolute(path).string());
                }
            }
        }
    }

    std::vector<picker::sv_signal_define> sv_pin(nlohmann::json &module_token, std::string &src_module_name)
    {
        PK_MESSAGE("want module: %s", src_module_name.c_str());

        // 解析module_token，将解析好的pin_name\pin_length\pin_type都push进pin数组并返回。
        std::map<std::string, std::string> parameter_var;
        std::vector<picker::sv_signal_define> pin;
        for (int j = 0; j < module_token.size(); j++)
            if (module_token[j]["tag"] == "module" && module_token[j + 1]["text"] == src_module_name) {
                std::string pin_type, pin_high, pin_low;
                for (int i = j + 2; i < module_token.size(); i++) {
                    PK_DEBUG("%s", module_token[i]["tag"].dump().c_str());
                    // Check if is parameter
                    if (module_token[i]["tag"] == "parameter") {
                        pin_type = "parameter";
                        continue;
                    }
                    // Record pin type
                    if (module_token[i]["tag"] == "input" || module_token[i]["tag"] == "output"
                        || module_token[i]["tag"] == "inout") {
                        pin_type = module_token[i]["tag"];
                        pin_high.clear();
                        pin_low.clear();
                        // Skip logic, wire, reg type. We export pins with logic type, these types are not needed yet.
                        i++;
                        if (module_token[i]["tag"] == "logic" || module_token[i]["tag"] == "wire"
                            || module_token[i]["tag"] == "reg") {
                            i++;
                        }
                        // Parse pin length
                        if (module_token[i]["tag"] == "[") {
                            while (module_token[++i]["tag"] != ":") pin_high += param_res(token_res(i));
                            while (module_token[++i]["tag"] != "]") pin_low += param_res(token_res(i));
                        } else {
                            i--;
                            pin_high = "-1";
                        }

                        continue;
                    }

                    if (pin_type == "parameter") {
                        if (module_token[i]["tag"] == "SymbolIdentifier") {
                            std::string parameter_name = module_token[i++]["text"];
                            std::string parameter_value;
                            while (module_token[++i]["tag"] != "," && module_token[i]["tag"] != ")") {
                                parameter_value += param_res(token_res(i));
                            }
                            parameter_var[parameter_name] = parameter_value;
                            PK_DEBUG("parameter_name: %s, parameter_value: %s", parameter_name.c_str(),
                                     parameter_value.c_str());
                        }
                        continue;
                    }

                    // lambda function to parse and calc pin length wich exprtk
                    exprtk::parser<double> parser;
                    exprtk::expression<double> expression;
                    auto calc_pin_length = [&](const std::string &exp) {
                        if (parser.compile(exp, expression)) {
                            PK_DEBUG("pin length expression: %s as %f", exp.c_str(), expression.value());
                            return expression.value();
                        } else {
                            PK_FATAL("Failed to parse pin length expression: %s .\n", exp.c_str());
                        }
                    };

                    // If is pin
                    sv_signal_define tmp_pin;
                    tmp_pin.logic_pin_type = pin_type;

                    if (module_token[i]["tag"] == "SymbolIdentifier") { // Noraml pin
                        tmp_pin.logic_pin = module_token[i]["text"];
                        if (pin_high != "-1") {
                            tmp_pin.logic_pin_hb = calc_pin_length(pin_high);
                            tmp_pin.logic_pin_lb = calc_pin_length(pin_low);
                        } else {
                            tmp_pin.logic_pin_hb = -1;
                        }
                        pin.push_back(tmp_pin);
                    } else if (module_token[i]["tag"] == ")") { // Escaped pin
                        goto module_out;
                    }
                }
            module_out:
                return pin;
            }
        pin.clear();
        return pin;
    }

    std::map<std::string, int> parse_mname_and_numbers(std::vector<std::string> &name_and_nums)
    {
        // eg: MouduleA,1,ModuleB,3,MouldeC,ModuleE,ModuleF
        std::string m_name = "";
        int num            = 1;
        std::map<std::string, int> ret;
        for (auto v : name_and_nums) {
            v   = picker::trim(v);
            num = 1;
            if (picker::str_start_with_digit(v)) {
                num = std::stoi(v);
                if (!m_name.empty()) {
                    ret[m_name] = num;
                    m_name      = "";
                } else {
                    PK_MESSAGE("Ignore num: %d, no matched Module find", num)
                }
            } else {
                vassert(!v.empty(), "Find empty Module name in --sname arg");
                if (!m_name.empty()) { ret[m_name] = num; }
                m_name = v;
            }
        }
        if (!m_name.empty()) ret[m_name] = 1;
        return ret;
    }

    // Quick text-based prefilter to reduce heavy syntax parsing invocations
    static bool file_may_contain_any_module(const std::string &filepath, const std::vector<std::string> &m_names)
    {
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) return false;
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.find("module") == std::string::npos) continue;
            for (const auto &m : m_names) {
                // Look for pattern: word-boundary 'module' + spaces + name + non-identifier boundary
                auto pos = line.find("module "+m);
                if (pos == std::string::npos) continue;
                // Check preceding boundary
                bool ok_prev = (pos == 0) || !std::isalnum(static_cast<unsigned char>(line[pos-1])) && line[pos-1] != '_';
                // After 'module ' + m
                size_t endpos = pos + std::string("module ").size() + m.size();
                bool ok_next = (endpos >= line.size()) || (!std::isalnum(static_cast<unsigned char>(line[endpos])) && line[endpos] != '_');
                if (ok_prev && ok_next) return true;
            }
        }
        return false;
    }

    std::map<std::string, nlohmann::json> match_module_with_file(std::vector<std::string> files,
                                                                 std::vector<std::string> m_names)
    {
        std::map<std::string, nlohmann::json> ret;
        for (auto f : files) {
            // If module list specified, quickly skip files that obviously do not contain any of them
            if (!m_names.empty()) {
                try {
                    if (!file_may_contain_any_module(f, m_names)) continue;
                } catch (...) {
                    // On any error in quick pass, fall back to full check to be safe
                }
            }
            std::string fpath = "/tmp/" + std::filesystem::path(f).filename().string() + std::string(lib_random_hash)
                                + picker::get_node_uuid() + ".json";
            std::string syntax_cmd =
                std::string(appimage::is_running_as_appimage() ? appimage::get_temporary_path() + "/usr/bin/" : "")
                + "verible-verilog-syntax --export_json --printtokens " + f + "> " + fpath;
            exec(syntax_cmd.c_str());
            auto mjson = nlohmann::json::parse(read_file(fpath));
            std::vector<std::string> module_list;
            auto module_token = mjson[f]["tokens"];
            for (int i = 0; i < module_token.size(); i++) {
                if (module_token[i]["tag"] == "module") module_list.push_back(module_token[i + 1]["text"]);
            }
            if (m_names.empty()) {
                ret[module_list.back()] = module_token;
                return ret;
            }
            for (auto m : m_names) {
                if (picker::contains(module_list, m)) {
                    PK_MESSAGE("find module: %s in file: %s", m.c_str(), f.c_str())
                    ret[m] = module_token;
                    // todo: can be optimized, only collect matched module tokens instead of all tokens in this file
                }
            }
            PK_DEBUG("rm -f %s", fpath.c_str());
            exec(("rm -f " + fpath).c_str());
            if (ret.size() == m_names.size()) break; // All found
        }
        for (auto m : m_names) { vassert(ret.count(m), "Module: " + m + " not find in input file"); }
        return ret;
    }

    int sv(picker::export_opts &opts, std::vector<picker::sv_module_define> &sv_module_result)
    {
        std::string dst_module_name = opts.target_module_name;
        std::vector<std::string> m_names;
        std::map<std::string, nlohmann::json> m_json;
        std::map<std::string, int> m_nums;

        if (opts.source_module_name_list.empty()) {
            if (opts.file.size() != 1)
                PK_FATAL("When module name not given (--sname),"
                         " can only parse one .v/.sv file (%d find!)",
                         (int)opts.file.size())
            m_json  = match_module_with_file(opts.file, opts.source_module_name_list);
            m_names = picker::key_as_vector(m_json);
            for (auto &v : m_names) { m_nums[v] = 1; }
        } else {
            // When top module is given, search it within both `file` and expanded `--fs/--filelist` entries
            m_nums  = parse_mname_and_numbers(opts.source_module_name_list);
            m_names = picker::key_as_vector(m_nums);

            std::vector<std::string> search_files = opts.file; // always include explicit files
            std::vector<std::string> extra_files;
            collect_verilog_from_filelists(opts.filelists, extra_files);

            // De-duplicate while preserving order
            std::unordered_set<std::string> seen;
            for (auto &f : search_files) seen.insert(std::filesystem::absolute(f).string());
            for (auto &f : extra_files) {
                auto af = std::filesystem::absolute(f).string();
                if (seen.insert(af).second) search_files.push_back(af);
            }

            m_json = match_module_with_file(search_files, m_names);
        }

        auto target_name = picker::join_str_vec(m_names, "_");
        for (auto &v : m_json) {
            picker::sv_module_define sv_module;
            sv_module.module_name = v.first;
            sv_module.module_nums = m_nums[sv_module.module_name];
            sv_module.pins        = sv_pin(v.second, sv_module.module_name);
            sv_module_result.push_back(sv_module);
        }

        opts.target_module_name = opts.target_module_name.size() == 0 ? target_name : opts.target_module_name;
        if (opts.target_dir.size() == 0 || opts.target_dir.back() == '/') {
            opts.target_dir += opts.target_module_name;
            PK_DEBUG("Set target dir to target module name: %s", opts.target_dir.c_str());
        }
        for (auto &m : sv_module_result) {
            PK_DEBUG("Module: %s, nums: %d", m.module_name.c_str(), m.module_nums);
            for (auto &p : m.pins) {
                PK_DEBUG("Pin: %s, type: %s, high: %d, low: %d", p.logic_pin.c_str(), p.logic_pin_type.c_str(),
                         p.logic_pin_hb, p.logic_pin_lb);
            }
        }
        PK_DEBUG("Target Module: %s", opts.target_module_name.c_str());

        return 0;
    }
}} // namespace picker::parser
