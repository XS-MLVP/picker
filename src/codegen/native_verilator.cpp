#include "codegen/native_verilator.hpp"

namespace picker { namespace native_verilator {
    // Need support the following functions:
    // - init code in construct function

    static const std::string temp_native_init = \
    "    this->native_singnals[\"{{PIN_NAME}}\"] = (uint64_t)(&((V{{TOP_MODULE_NAME}} *)(this->top))->rootp->{{__LIB_DPI_FUNC_NAME_HASH__}}_{{PIN_NAME}});\n";

    std::string render_signal_init(std::vector<picker::sv_signal_define> &external_pin, picker::export_opts &opts){
        std::string ret = "    this->native_singnal_enable=true;\n";
        inja::Environment env;
        for (int i = 0; i < external_pin.size(); i++) {
            nlohmann::json value;
            value["PIN_NAME"] = external_pin[i].logic_pin;
            value["TOP_MODULE_NAME"] = opts.target_module_name;
            value["__LIB_DPI_FUNC_NAME_HASH__"] = std::string(lib_random_hash);
            ret += env.render(temp_native_init, value);
        }
        return ret;
    }

    void native_signal(picker::export_opts &opts, std::vector<picker::sv_signal_define> external_pin,
                std::vector<picker::sv_signal_define> internal_signal, nlohmann::json &result){
        result["__NATIVE_SIGNAL_INIT__"] = render_signal_init(external_pin, opts);
    }
}}
