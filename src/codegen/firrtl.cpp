#include "codegen/firrtl.hpp"
#include "codegen/render_tags.hpp"


namespace picker { namespace codegen {
std::vector<picker::sv_signal_define> gen_firrtl_param(nlohmann::json &global_render_data,
                                                       const std::vector<picker::sv_module_define> &sv_module_result,
                                                       const std::vector<picker::sv_signal_define> &internal_signal,
                                                       nlohmann::json &signal_tree_json,
                                                       const std::string &wave_file_name, const std::string &simulator, SignalAccessType rw_type){

        global_render_data[tags::wave_file_name]         = "__GISM_NOT_SUPPORT___WAVE_FILE_NAME__";
        global_render_data[tags::dump_var_options]       = "__GISM_NOT_SUPPORT___DUMP_VAR_OPTIONS__";
        global_render_data[tags::trace]                  = "__GISM_NOT_SUPPORT___TRACE__";
        global_render_data[tags::logic_pin_declaration]  = "__GISM_NOT_SUPPORT___LOGIC_PIN_DECLARATION__";
        global_render_data[tags::sv_dump_wave]           = "__GISM_NOT_SUPPORT___SV_DUMP_WAVE__";
        global_render_data[tags::wire_pin_declaration]   = "__GISM_NOT_SUPPORT___WIRE_PIN_DECLARATION__";
        global_render_data[tags::inner_modules]          = "__GISM_NOT_SUPPORT___INNER_MODULES__";
        global_render_data[tags::dpi_function_export]    = "__GISM_NOT_SUPPORT___DPI_FUNCTION_EXPORT__";
        global_render_data[tags::dpi_function_implement] = "__GISM_NOT_SUPPORT___DPI_FUNCTION_IMPLEMENT__";
        global_render_data[tags::extend_sv]              = "__GISM_NOT_SUPPORT___EXTEND_SV__";
        global_render_data[tags::lib_dpi_func_name_hash] = "__GISM_NOT_SUPPORT___LIB_DPI_FUNC_NAME_HASH__";
        global_render_data[tags::signal_tree]                 = "__GISM_NOT_SUPPORT__signal_tree";
        assert(sv_module_result.size() == 1);
        return sv_module_result[0].pins;
    }

// end of namespace picker::codegen
};}
