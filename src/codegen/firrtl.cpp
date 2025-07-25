#include <bits/stdc++.h>
#include "codegen/firrtl.hpp"


namespace picker { namespace codegen {
std::vector<picker::sv_signal_define> gen_firrtl_param(nlohmann::json &global_render_data,
                                                       const std::vector<picker::sv_module_define> &sv_module_result,
                                                       const std::vector<picker::sv_signal_define> &internal_signal,
                                                       nlohmann::json &signal_tree_json,
                                                       const std::string &wave_file_name, const std::string &simulator, SignalAccessType rw_type){
        std::vector<picker::sv_signal_define> ret;

        global_render_data["__WAVE_FILE_NAME__"]   = "wave_file_name";
        global_render_data["__DUMP_VAR_OPTIONS__"] = "dum_var_options";
        global_render_data["__TRACE__"] = "OFF";
        global_render_data["__LOGIC_PIN_DECLARATION__"]  = "__LOGIC_PIN_DECLARATION__";
        global_render_data["__SV_DUMP_WAVE__"]           = "sv_dump_wave";
        global_render_data["__WIRE_PIN_DECLARATION__"]   = "wire";
        global_render_data["__INNER_MODULES__"]          = "inner_modules";
        global_render_data["__DPI_FUNCTION_EXPORT__"]    = "dpi_export";
        global_render_data["__DPI_FUNCTION_IMPLEMENT__"] = "dpi_impl";
        global_render_data["__EXTEND_SV__"]              = "extend_sv";
        global_render_data["__LIB_DPI_FUNC_NAME_HASH__"] = "std::string(lib_random_hash)";
        global_render_data["__SIGNAL_TREE__"]            = "signal_tree";
        return ret;
    }

// end of namespace picker::codegen
};}
