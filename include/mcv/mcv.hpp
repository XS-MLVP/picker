
#ifndef __MCV_MAIN__
#define __MCV_MAIN__

#include <iostream>
#include <vector>
#include <cstdio>
#include <unistd.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "sys/time.h"
#include <cppast/libclang_parser.hpp>


inline long time(){
    return clock();
}


inline std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}


inline std::string& trim(std::string &s){
    if (s.empty()){
        return s;
    }
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    s.erase(0,s.find_first_not_of("\r"));
    s.erase(s.find_last_not_of("\r") + 1);
    s.erase(0,s.find_first_not_of("\n"));
    s.erase(s.find_last_not_of("\n") + 1);
    return s;
}


inline std::vector<std::string> strsplit(std::string str, std::string s=" ") {
    std::vector<std::string> ret;
    int start = 0;
    int end = str.find(s);
    while (end != -1) {
        auto sub = str.substr(start, end - start);
        trim(sub);
        ret.push_back(sub);
        start = end + s.size();
        end = str.find(s, start);
    }
    auto sub = str.substr(start, end - start);
    trim(sub);
    ret.push_back(sub);
    return ret;
}


template <typename T>
inline bool contians(std::vector<T> v, T a) {
    for(T x :v){
        if (x == a)return true;
    }
    return false;
}


inline bool file_exists(std::string f) {
    return access(f.c_str(), F_OK) == 0;
}


inline std::string exec_result(std::string cmd, int index){
    auto result = strsplit(exec(cmd.c_str()));
    if (result.size() <= index){
        return "";
    }
    return result[index];
}


extern bool is_debug;
#define OUTPUT(o, fmt, ...) {fprintf(o, fmt, ##__VA_ARGS__);}
#define MESSAGE(fmt,...) {OUTPUT(stdout, fmt, ##__VA_ARGS__);OUTPUT(stdout,"%s\n", "");}
#define ERROR(fmt,...) {OUTPUT(stderr, fmt, ##__VA_ARGS__)}
#define DEBUG(fmt,...) {if(is_debug){OUTPUT(stderr, "%s", "debug> ");OUTPUT(stderr, fmt, ##__VA_ARGS__);OUTPUT(stderr, "%s\n", "")}}
#define FATAL(fmt,...) {OUTPUT(stderr, fmt, ##__VA_ARGS__);exit(-1);}


inline void assert(bool c, std::string msg = "") {
    if (!c){
        FATAL("Assert error: %s\n", msg.c_str());
    }
}

// export functions
void gen_python(std::unique_ptr<cppast::cpp_file> &cppfile, std::string target_dir);

#endif
