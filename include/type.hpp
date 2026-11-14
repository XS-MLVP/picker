#pragma once
#include <string>
#include <memory>
#include <utility>
#include <vector>

namespace picker {


enum class SignalAccessType : int {
    DPI = 0,
    MEM_DIRECT = 1,
};

typedef struct main_opts {
    bool version;
    bool show_default_template_path;
    bool show_xcom_lib_location_cpp;
    bool show_xcom_lib_location_java;
    bool show_xcom_lib_location_scala;
    bool show_xcom_lib_location_golang;
    bool show_xcom_lib_location_python;
    bool show_xcom_lib_location_lua;
    bool check;
} main_opts;

typedef struct export_opts {
    std::vector<std::string> file;
    std::vector<std::string> filelists;
    std::string sim;
    std::string language;
    std::string source_dir;
    std::string target_dir;
    std::vector<std::string> source_module_name_list;
    std::string target_module_name;
    std::string internal;
    bool checkpoints;
    bool vpi;
    std::string frequency;
    std::string wave_file_name;
    bool coverage;
    std::string verdi_mode;  // legacy: -P novas.tab, modern: -debug_access+all
    std::string vflag;
    std::string cflag;
    std::vector<std::string> sflag;
    bool verbose;
    bool example;
    bool autobuild;
    int build_threads;
    bool cp_lib;
    SignalAccessType rw_type;
    std::vector<std::string> transaction;
    std::string rname;
    std::string config_model_id; // Identity shared by the generated model and its one configuration database.
    // --rtl-param NAME=VALUE entries; evaluated by the RTL frontend and pinned
    // in the generated wrapper. Never forwarded to the simulator.
    std::vector<std::string> rtl_param;
    // --rtl-define NAME[=VALUE] entries; forwarded to both the RTL frontend and
    // the simulator, because macros cannot be evaluated once and reused.
    std::vector<std::string> rtl_define;
} export_opts;

typedef struct pack_opts {
    std::vector<std::string> files;
    std::vector<std::string> rename;
    std::vector<std::string> filelist;
    std::string name;
    bool example;
    bool force;
    bool generate_dut;
    std::string from_rtl_file;  // RTL source file path for auto-generating transaction
} pack_opts;

typedef struct sv_signal_define {
    std::string logic_pin;
    std::string logic_pin_type;
    int logic_pin_hb;
    int logic_pin_lb;
    // Original fixed unpacked ranges, outermost first; retained on lowered elements.
    std::vector<std::pair<int, int>> unpacked_dimensions;
    // Array connections require equivalent element types (bit vs logic, signedness).
    std::string unpacked_element_type = "logic";
    // Lowered elements keep their parent bounds and RTL indices, independent of logic_pin.
    std::string array_name;
    std::vector<int> array_indices;
    // Recursive type description for struct and multidimensional packed ports.
    // Lowered leaves retain their original public path and their root layout.
    std::shared_ptr<const std::string> type_layout;
    std::string aggregate_name;
    std::string signal_path;
    // Compiler expressions for a homogeneous unpacked field group. There is
    // one shared binding per field group, not one native registration per leaf.
    std::string binding_native;
    std::vector<std::string> binding_strides;
} sv_signal_define;

// A parameter override for the generated wrapper instantiation. The value is
// the elaborated constant rendered as a self-contained SystemVerilog literal,
// so the generated code does not depend on the original default expression.
typedef struct sv_parameter_define {
    std::string name;
    std::string value;
} sv_parameter_define;

typedef struct sv_module_define {
    std::string module_name;
    int module_nums;
    std::vector<sv_signal_define> pins;
    // Overridable port-list parameters; body parameters and localparams are
    // not representable in a named parameter assignment.
    std::vector<sv_parameter_define> parameters;
    // Compact semantic metadata. Layouts reference interned types rather than
    // copying a type tree for every variable or array element.
    std::vector<std::pair<std::string, std::string>> native_layouts;
    std::vector<std::pair<std::string, std::string>> native_types;
} sv_module_define;

typedef struct uvm_parameter {
    std::string name;
    int byte_count;
    int bit_count;
    int is_macro;
    std::string macro_name;
    std::string current_index;

    // Default constructor with sensible defaults
    uvm_parameter()
        : name("")
        , byte_count(1)
        , bit_count(1)
        , is_macro(0)
        , macro_name("")
        , current_index("0")
    {}
} uvm_parameter;

typedef struct uvm_transaction_define {
    std::string name;
    std::string filepath;
    std::string version;
    std::string data_now;
    std::vector<uvm_parameter> parameters;
} uvm_transaction_define;

struct cpp_native_group {
    std::string path;
    std::string member;
    int width = 0;
    std::vector<int> dimensions;
    std::vector<std::string> strides;
};

typedef struct cpp_variableInfo {
    std::string name;
    std::string type;
    int width      = 0;
    int array_size = 0;
    int64_t offset = 0;
    std::vector<int> array_dimensions;
    std::string layout; // interned type id, not an expanded layout
    std::vector<cpp_native_group> native_groups;
    std::string view_error;
    std::string public_name; // Trace-proven logical alias, if different from native storage name.
} cpp_variableInfo;

} // namespace picker
