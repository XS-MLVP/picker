
#ifndef __MCV_MAIN__
#define __MCV_MAIN__

#include <iostream>
#include <vector>
#include <cstdio>

inline std::vector<std::string> strsplit(std::string str, std::string s=" ") {
    std::vector<std::string> ret;
    int start = 0;
    int end = str.find(s);
    while (end != -1) {
        ret.push_back(str.substr(start, end - start));
        start = end + s.size();
        end = str.find(s, start);
    }
    ret.push_back(str.substr(start, end - start));
    return ret;
}

template <typename T>
inline bool contians(std::vector<T> v, T a) {
    for(T x :v){
        if (x == a)return true;
    }
    return false;
}

std::string& trim(std::string &s){
    if (s.empty()){
        return s;
    }
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

#define OUTPUT(o, fmt, ...) {fprintf(o, fmt, ##__VA_ARGS__);}
#define MESSAGE(fmt,...) {OUTPUT(stderr, fmt, ##__VA_ARGS__)}

#endif
