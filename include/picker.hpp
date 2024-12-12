
#pragma once

#include <bits/stdc++.h>
#include <sys/time.h>
#include <chrono>
#include <regex>
#include <set>
#include <unistd.h>
#include <filesystem>
#include <functional>
#include "type.hpp"
#include "json.hpp"
#include "inja.hpp"
#include "CLI11.hpp"
#include "version.hpp"
#include "codegen/cpp.hpp"
#include "codegen/python.hpp"
#include "codegen/java.hpp"
#include "codegen/scala.hpp"
#include "codegen/golang.hpp"
#include "codegen/sv.hpp"
#include "codegen/lib.hpp"
#include "parser/sv.hpp"
#include "parser/internalcfg.hpp"

namespace picker {

extern bool is_debug;
extern char *lib_random_hash;

#define PK_OUTPUT(o, fmt, ...)                                                                                         \
    {                                                                                                                  \
        fprintf(o, fmt, ##__VA_ARGS__);                                                                                \
    }
#define PK_MESSAGE(fmt, ...)                                                                                           \
    {                                                                                                                  \
        PK_OUTPUT(stdout, fmt, ##__VA_ARGS__);                                                                         \
        PK_OUTPUT(stdout, "%s\n", "");                                                                                 \
    }
#define PK_ERROR(fmt, ...)                                                                                             \
    {                                                                                                                  \
        PK_OUTPUT(stderr, "%s", "\033[31m")                                                                            \
        PK_OUTPUT(stderr, fmt, ##__VA_ARGS__)                                                                          \
        PK_OUTPUT(stderr, "%s\n", "\033[0m")                                                                           \
    }
#define PK_DEBUG(fmt, ...)                                                                                             \
    {                                                                                                                  \
        if (is_debug) {                                                                                                \
            PK_OUTPUT(stderr, "%s", "debug> ");                                                                        \
            PK_OUTPUT(stderr, fmt, ##__VA_ARGS__);                                                                     \
            PK_OUTPUT(stderr, "%s\n", "")                                                                              \
        }                                                                                                              \
    }
#define PK_FATAL(fmt, ...)                                                                                             \
    {                                                                                                                  \
        PK_OUTPUT(stderr, "%sFatal: ", "\033[31m")                                                                     \
        PK_OUTPUT(stderr, fmt, ##__VA_ARGS__);                                                                         \
        PK_OUTPUT(stderr, "%s\n", "\033[0m")                                                                           \
        exit(-1);                                                                                                      \
    }

inline void vassert(bool c, std::string msg = "")
{
    if (!c) { PK_FATAL("Assert error: %s", msg.c_str()); }
}

inline long vtime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

inline std::string fmtnow(std::string fmt = "%Y-%m-%d %H:%M:%S")
{
    std::time_t t = std::time(nullptr);
    std::tm tm    = *std::localtime(&t);
    std::stringstream buffer;
    buffer << std::put_time(&tm, fmt.c_str());
    return buffer.str();
}

inline std::string exec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) { throw std::runtime_error("popen() failed!"); }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) { result += buffer.data(); }
    return result;
}

inline std::string trim(std::string s, std::string p)
{
    if (s.empty()) { return s; }
    s.erase(0, s.find_first_not_of(p));
    s.erase(s.find_last_not_of(p) + p.size());
    return s;
}

inline std::string ltrim(std::string s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    return s;
}

inline std::string rtrim(std::string s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

inline std::string trim(std::string s)
{
    return rtrim(ltrim(s));
}

inline std::vector<std::string> strsplit(std::string str, std::string s = " ")
{
    std::vector<std::string> ret;
    int start = 0;
    int end   = str.find(s);
    while (end != -1) {
        auto sub = str.substr(start, end - start);
        trim(sub);
        ret.push_back(sub);
        start = end + s.size();
        end   = str.find(s, start);
    }
    auto sub = str.substr(start, end - start);
    trim(sub);
    ret.push_back(sub);
    return ret;
}

inline std::string lower_case(std::string input)
{
    auto str = input;
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
}

inline std::string capitalize_first_letter(const std::string &input)
{
    if (input.empty()) { return input; }

    std::string result = input;
    result[0]          = std::toupper(result[0]);
    return result;
}

inline std::map<std::string, std::string> get_default_confilct_map()
{
    std::map<std::string, std::string> pin_map;
    std::vector<std::string> conflict_pin = {"xclock",      "xport",   "dut",        "initclock",
                                             "step",        "stepris", "stepfal",    "setwaveform",
                                             "setcoverage", "finish",  "refreshcomb"};
    for (auto &p : conflict_pin) { pin_map[p] = "pin_"; }
    return pin_map;
}

inline std::string fix_conflict_pin_name(const std::string &in, std::map<std::string, std::string> &pin_map,
                                         bool cap_first = false)
{
    auto input = lower_case(in);
    if (input.empty()) { PK_FATAL("input pin name is empty"); }
    // not find in conflict map
    if (pin_map.find(input) == pin_map.end()) {
        if (cap_first) { return capitalize_first_letter(in); }
        return in;
    }
    std::string result = pin_map[input] + in;
    if (cap_first) { result = capitalize_first_letter(result); }
    return result;
}

inline std::string strsplit(std::string str, std::initializer_list<std::string> s, bool front = true)
{
    auto ret = str;
    for (auto &v : s) {
        if (front) {
            ret = strsplit(ret, v).front();
        } else {
            ret = strsplit(ret, v).back();
        }
    }
    return ret;
}

inline bool strconatins(std::string str, std::string subs)
{
    return str.find(subs) != std::string::npos;
}

inline std::string streplace(std::string str, std::string s, std::string t = "")
{
    std::string input = str;
    size_t index      = 0;
    while (true) {
        index = input.find(s, index);
        if (index == std::string::npos) break;
        input.replace(index, s.length(), t);
        index += t.length();
    }
    return input;
}

inline std::string streplace(std::string str, std::initializer_list<std::string> s, std::string t = "")
{
    auto ret = str;
    for (auto &v : s) { ret = streplace(ret, v, t); }
    return ret;
}

template <typename T>
inline bool contians(std::vector<T> v, T a)
{
    for (T x : v) {
        if (x == a) return true;
    }
    return false;
}

template <typename T, typename F>
inline std::vector<T> key_as_vector(std::map<T, F> &input)
{
    std::vector<T> keys;
    for (const auto &pair : input) { keys.push_back(pair.first); }
    return keys;
}

inline std::string join_str_vec(const std::vector<std::string> &strings, const std::string &delimiter)
{
    if (strings.empty()) return "";
    std::string result = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) { result += delimiter + strings[i]; }
    return result;
}

inline bool file_exists(std::string f)
{
    return access(f.c_str(), F_OK) == 0;
}

inline std::string exec_result(std::string cmd, int index)
{
    auto result = strsplit(exec(cmd.c_str()));
    if (result.size() <= index) { return ""; }
    return result[index];
}

inline std::string first_upercase(std::string input)
{
    std::string ret = input;
    ret[0]          = std::toupper(input[0]);
    return ret;
}

inline std::string base_name(std::string const &path, bool with_subfix = true)
{
    auto fname = path.substr(path.find_last_of("/\\") + 1);
    if (with_subfix) { return fname; }
    return strsplit(fname, ".")[0];
}

inline std::string get_path(std::string str)
{
    auto found = str.find_last_of("/\\");
    return str.substr(0, found);
}

inline std::string normal_path(std::string path)
{
    return std::filesystem::absolute(path).lexically_normal().string();
}

inline std::string path_join(std::initializer_list<std::string> args)
{
    std::string path, value;
    auto size = args.size();
    for (auto p : args) {
        if (path.empty()) {
            path = p;
            path.erase(path.find_last_not_of("/\\") + 1);
            continue;
        }
        auto v = trim(p, "/");
        v      = trim(v, "\\");
        path.append("/" + v);
    }
    return normal_path(path);
}

inline std::string current_path()
{
    return std::string(get_current_dir_name());
}

template <typename... Args>
std::string sfmt(const std::string &format, Args... args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(),
                       buf.get() + size - 1); // We don't want the '\0' inside
}

inline std::string version()
{
    return "v0.1";
}

inline std::string find_file(std::vector<std::string> fs)
{
    for (auto f : fs) {
        if (file_exists(f)) {
            PK_DEBUG("find file: %s", f.c_str());
            return f;
        }
    }
    vassert(false, "search file fail");
    return "";
}

inline bool write_file(std::string fname, std::string text)
{
    PK_MESSAGE("Write file: %s", fname.c_str());
    std::ofstream ofile(fname, std::ios::trunc);
    vassert(ofile.is_open(), "write file(" + fname + ") error");
    ofile << text;
    ofile.close();
    return true;
}

inline std::string read_file(std::string fname)
{
    std::ifstream ifile(fname, std::ios::in | std::ios::binary);
    if (ifile) {
        std::string content;
        ifile.seekg(0, std::ios::end);
        content.resize(ifile.tellg());
        ifile.seekg(0, std::ios::beg);
        ifile.read(&content[0], content.size());
        ifile.close();
        return content;
    }
    throw(errno);
}

inline std::string read_params(std::string fname)
{
    PK_MESSAGE("Read params from: %s", fname.c_str());
    std::ifstream ifile(fname);
    std::string ret;
    if (ifile.is_open()) {
        std::string line_txt;
        while (std::getline(ifile, line_txt)) {
            line_txt = trim(strsplit(line_txt, "#").front());
            if (line_txt.empty()) { continue; }
            if (ret.empty()) {
                ret = line_txt;
            } else {
                ret = ret + " " + line_txt;
            }
        }
    }
    return ret;
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>

inline std::string get_executable_path()
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    return std::string(buffer);
}

#elif __APPLE__
#include <mach-o/dyld.h>

inline std::string get_executable_path()
{
    char buffer[1024];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
        return std::string(buffer);
    else
        return std::string("");
}

#elif __linux__
#include <unistd.h>

inline std::string get_executable_path()
{
    char buffer[1024];
    ssize_t len = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::string(buffer);
    } else {
        return std::string("");
    }
}

#endif

inline bool str_start_with(std::string str, std::string prefix)
{
    return str.find(prefix) == 0;
}

inline bool str_start_with_digit(const std::string &str)
{
    if (str.empty()) return false;
    return std::isdigit(str[0]);
}

inline std::string get_target_path_from(std::string base, std::vector<std::string> paths)
{
    auto path = get_executable_path();
    for (auto &l : paths) {
        auto p = std::string(l);
        if (!str_start_with(p, "/")) { p = path_join({base, p}); }
        if (file_exists(p)) { return p; }
    }
    return "";
}

inline std::string get_target_path_from(std::string base, std::initializer_list<std::string> paths)
{
    return get_target_path_from(base, std::vector<std::string>(paths));
}

inline std::string get_xcomm_location()
{
    auto path = get_executable_path();
    return get_target_path_from(path, {"../../dependence/xcomm",  // 1. search in bulid dir (for dev)
                                       "./picker",                // 2. search in current dir
                                       "../../share/picker",      // 3. search in share dir (for install)
                                       "/usr/local/share/picker", // 4. search in /usr/local/share
                                       "/usr/share/picker",       // 5. search in /usr/share
                                       "/etc/picker"});           // 6. search in /etc
}

inline std::string get_xcomm_lib(std::string lib_name, std::string &message)
{
    auto path = get_xcomm_location();
    if (path.empty()) {
        message = "xcomm lib not found";
        return "";
    }
    auto lib = path_join({path, lib_name});
    if (file_exists(lib)) { return lib; }
    message = lib + " not found";
    return "";
}

inline std::string get_template_path()
{
    auto path = get_executable_path();
    auto tmp  = get_target_path_from(path, {"../../../template",                // 1. search in source dir (for dev)
                                           "./template",                       // 2. search in current dir
                                           "../../share/picker/template",      // 3. search in share dir (for install)
                                           "/usr/local/share/picker/template", // 4. search in /usr/local/share
                                           "/etc/picker/template"});           // 5. search in /etc
    if (!tmp.empty()) { return tmp; }
    PK_FATAL("template not found, please check the installation or mannualy set the source dir path");
}
inline std::string extract_name(const std::string &input, char delimiter, int isFirst)
{
    size_t pos = input.find(delimiter);
    if (pos != std::string::npos) {
        if (isFirst == 1) {
            return input.substr(0, pos);
        } else {
            return input.substr(pos + 1);
        }
    }
    return input; // 如果没有找到分隔符，则返回整个字符串
}

inline std::string get_env(std::string key, std::string default_value = "")
{
    const char *value = std::getenv(key.c_str());
    if (value) {
        return std::string(value);
    } else {
        return default_value;
    }
}

inline std::string getCurrentTime()
{
    auto now          = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm    = *std::localtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

inline std::string get_hostname()
{
    char hostname[1024];
    gethostname(hostname, sizeof(hostname));
    return std::string(hostname);
}

inline std::string get_node_uuid()
{
    std::hash<std::string> hasher;
    return std::to_string(hasher(get_hostname() + std::to_string(getpid())));
}

} // namespace picker