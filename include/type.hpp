#pragma once
#include <bits/stdc++.h>

namespace picker {
typedef struct exports_opts {
    std::string file;
    std::string filelist;
    std::string sim;
    std::string language;
    std::string source_dir;
    std::string target_dir;
    std::string source_module_name;
    std::string target_module_name;
    std::string internal;
    std::string frequency;
    std::string wave_file_name;
    bool coverage;
    std::string vflag;
    std::string cflag;
    bool verbose;
    bool version;
    bool example;
    bool autobuild;
} exports_opts;

typedef struct sv_signal_define {
    std::string logic_pin;
    std::string logic_pin_type;
    int logic_pin_hb;
    int logic_pin_lb;
} sv_signal_define;
} // namespace picker
