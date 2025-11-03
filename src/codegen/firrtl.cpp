#include "codegen/firrtl.hpp"


namespace picker { namespace codegen {
std::vector<picker::sv_signal_define> gen_firrtl_param(nlohmann::json &global_render_data,
                                                       const std::vector<picker::sv_module_define> &sv_module_result,
                                                       const std::vector<picker::sv_signal_define> &internal_signal,
                                                       nlohmann::json &signal_tree_json,
                                                       const std::string &wave_file_name, const std::string &simulator, SignalAccessType rw_type){

        global_render_data["__WAVE_FILE_NAME__"]         = "__GISM_NOT_SUPPORT___WAVE_FILE_NAME__";
        global_render_data["__DUMP_VAR_OPTIONS__"]       = "__GISM_NOT_SUPPORT___DUMP_VAR_OPTIONS__";
        global_render_data["__TRACE__"]                  = "__GISM_NOT_SUPPORT___TRACE__";
        global_render_data["__LOGIC_PIN_DECLARATION__"]  = "__GISM_NOT_SUPPORT___LOGIC_PIN_DECLARATION__";
        global_render_data["__SV_DUMP_WAVE__"]           = "__GISM_NOT_SUPPORT___SV_DUMP_WAVE__";
        global_render_data["__WIRE_PIN_DECLARATION__"]   = "__GISM_NOT_SUPPORT___WIRE_PIN_DECLARATION__";
        global_render_data["__INNER_MODULES__"]          = "__GISM_NOT_SUPPORT___INNER_MODULES__";
        global_render_data["__DPI_FUNCTION_EXPORT__"]    = "__GISM_NOT_SUPPORT___DPI_FUNCTION_EXPORT__";
        global_render_data["__DPI_FUNCTION_IMPLEMENT__"] = "__GISM_NOT_SUPPORT___DPI_FUNCTION_IMPLEMENT__";
        global_render_data["__EXTEND_SV__"]              = "__GISM_NOT_SUPPORT___EXTEND_SV__";
        global_render_data["__LIB_DPI_FUNC_NAME_HASH__"] = "__GISM_NOT_SUPPORT___LIB_DPI_FUNC_NAME_HASH__";
        global_render_data["__SIGNAL_TREE__"]            = "__GISM_NOT_SUPPORT___SIGNAL_TREE__";
        assert(sv_module_result.size() == 1);
        return sv_module_result[0].pins;
    }

// end of namespace picker::codegen
};}
