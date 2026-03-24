#include "picker.hpp"
#include "parser/sv.hpp"
#include "parser/slang_sv.hpp"

#include <unordered_set>

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

    int sv(picker::export_opts &opts, std::vector<picker::sv_module_define> &sv_module_result)
    {
        std::vector<std::string> m_names;
        std::map<std::string, int> m_nums;

        if (opts.source_module_name_list.empty()) {
            parse_sv_with_slang(opts, m_nums, m_names, sv_module_result);
        } else {
            m_nums  = parse_mname_and_numbers(opts.source_module_name_list);
            parse_sv_with_slang(opts, m_nums, m_names, sv_module_result);
        }

        auto target_name = picker::join_str_vec(m_names, "_");
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
