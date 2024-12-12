#include <bits/stdc++.h>
#include "codegen/cpp.hpp"

namespace picker {

std::string replace_all(std::string str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

namespace codegen {

    namespace cxx {
        static const std::string xdata_declaration_template = "    XData {{pin_uniq_name}};\n";
        static const std::string xdata_reinit_template =
            "    this->{{pin_uniq_name}}.ReInit({{logic_pin_length}}, IOType::{{logic_pin_type}}, \"{{logic_pin}}\");\n";
        static const std::string xdata_bindrw_template =
            "    this->{{pin_uniq_name}}.BindDPIPtr(this->dut->GetDPIHandle((char *)\"{{pin_func_name}}\", 0), this->dut->GetDPIHandle((char *)\"{{pin_func_name}}\", 1));\n";
        static const std::string xdata_bind_onlyr_template =
            "    this->{{pin_uniq_name}}.BindDPIPtr(this->dut->GetDPIHandle((char *)\"{{pin_func_name}}\", 0), 0);\n";
        static const std::string xport_add_template =
            "    this->xport.Add(this->{{pin_uniq_name}}.mName, this->{{pin_uniq_name}});\n";
        static const std::string comment_template = "    {{logic_pin_type}} {{logic_pin_length}} {{logic_pin}}\n";
        static const std::string xport_cascaded_dec_template = "    XPort {{port_name}};\n";
        static const std::string xport_cascaded_sgn_template =
            "    this->{{port_name}} = this->xport.NewSubPort(\"{{prefix_key}}_\");\n";

#define BIND_DPI_RW                                                                                                    \
    if (pin[i].logic_pin_hb == -1) {                                                                                   \
        data["logic_pin_length"] = 0;                                                                                  \
        data["read_func_type"]   = "(void (*)(void*))";                                                                \
        data["write_func_type"]  = "(void (*)(const unsigned char))";                                                  \
    } else {                                                                                                           \
        data["logic_pin_length"] = pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;                                      \
        data["read_func_type"]   = "(void (*)(void*))";                                                                \
        data["write_func_type"]  = "(void (*)(const void*))";                                                          \
    }

        /// @brief Export external pin for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_external_pin(std::vector<picker::sv_signal_define> pin, std::string &xdata_declaration,
                                 std::string &xdata_reinit, std::string &xdata_bindrw, std::string &xport_add,
                                 std::string &comments)
        {
            inja::Environment env;
            nlohmann::json data;
            auto pin_map = picker::get_default_confilct_map();
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"]  = replace_all(pin[i].logic_pin, ".", "_");
                data["pin_uniq_name"]  = picker::fix_conflict_pin_name(data["pin_func_name"], pin_map, false);

                // Set 0 for 1bit singal or hb-lb+1 for vector signal for cpp
                // render
                data["logic_pin_type"] = first_upercase(pin[i].logic_pin_type);

                BIND_DPI_RW;
                data["logic_pin_length"] = pin[i].logic_pin_hb == -1 ? // means not vector
                                               0 :
                                               pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;

                xdata_declaration = xdata_declaration + env.render(xdata_declaration_template, data);
                xdata_reinit      = xdata_reinit + env.render(xdata_reinit_template, data);
                xdata_bindrw      = xdata_bindrw + env.render(xdata_bindrw_template, data);
                xport_add         = xport_add + env.render(xport_add_template, data);
            }
        }

        /// @brief Export internal signal for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_internal_signal(std::vector<picker::sv_signal_define> pin, std::string &xdata_declaration,
                                    std::string &xdata_reinit, std::string &xdata_bindrw, std::string &xport_add,
                                    std::string &comments)
        {
            inja::Environment env;
            nlohmann::json data;
            auto pin_map = picker::get_default_confilct_map();
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"]  = replace_all(pin[i].logic_pin, ".", "_");
                data["pin_uniq_name"]  = picker::fix_conflict_pin_name(data["pin_func_name"], pin_map, false);

                // Set empty or [hb:lb] for verilog render
                data["logic_pin_length"] = pin[i].logic_pin_hb == -1 ? "" :
                                                                       "[" + std::to_string(pin[i].logic_pin_hb) + ":"
                                                                           + std::to_string(pin[i].logic_pin_lb) + "]";

                comments = comments + env.render(comment_template, data);

                // Set 0 for 1bit singal or hb-lb+1 for vector signal for cpp
                // render
                BIND_DPI_RW;
                data["logic_pin_type"] = "Output";

                xdata_declaration = xdata_declaration + env.render(xdata_declaration_template, data);
                xdata_reinit      = xdata_reinit + env.render(xdata_reinit_template, data);
                xdata_bindrw      = xdata_bindrw + env.render(xdata_bind_onlyr_template, data);
                xport_add         = xport_add + env.render(xport_add_template, data);
            }
        };

        void render_cascaded_signals(std::string prefix, nlohmann::json &signal_tree_json,
                                     std::string &cascaded_ports_dec, std::string &cascaded_ports_sgn)
        {
            nlohmann::json data;
            inja::Environment env;
            for (auto &[key, port] : signal_tree_json.items()) {
                if (port.contains("_")) { // ignore leaf node
                    continue;
                } else {
                    std::string port_name = prefix.empty() ? key : prefix + "_" + key;
                    data["port_name"]     = port_name;
                    data["prefix_key"]    = port_name;
                    cascaded_ports_dec += env.render(xport_cascaded_dec_template, data);
                    cascaded_ports_sgn += env.render(xport_cascaded_sgn_template, data);
                    render_cascaded_signals(port_name, port, cascaded_ports_dec, cascaded_ports_sgn);
                }
            }
        }

    } // namespace cxx

    void cpp(picker::export_opts &opts, std::vector<picker::sv_signal_define> external_pin,
             std::vector<picker::sv_signal_define> internal_signal, nlohmann::json &signal_tree_json)
    {
        //
        std::string src_dir         = opts.source_dir + "/cpp";
        std::string dst_dir         = opts.target_dir + "/cpp";
        std::string dst_module_name = opts.target_module_name;

        // Codegen Buffers
        std::string pin_connect, logic, wire, comments, dpi_export, dpi_impl, xdata_declaration, xdata_reinit,
            xdata_bindrw, xport_add, cascaded_signals_dec, cascaded_signals_sgn;

        // Generate External Pin
        cxx::render_external_pin(external_pin, xdata_declaration, xdata_reinit, xdata_bindrw, xport_add, comments);
        // Generate Internal Signal
        cxx::render_internal_signal(internal_signal, xdata_declaration, xdata_reinit, xdata_bindrw, xport_add,
                                    comments);
        // Generate Cascaded Ports
        cxx::render_cascaded_signals("", signal_tree_json, cascaded_signals_dec, cascaded_signals_sgn);

        std::string erro_message;
        auto cpplib_location = picker::get_xcomm_lib("lib", erro_message);
        if (cpplib_location.empty()) { PK_FATAL("%s\n", erro_message.c_str()); }

        auto include_location = picker::get_xcomm_lib("include", erro_message);
        if (include_location.empty()) { PK_FATAL("%s\n", erro_message.c_str()); }

        // Set global render data
        nlohmann::json data;
        data["__XSPCOMM_LIB__"]     = cpplib_location;
        data["__XSPCOMM_INCLUDE__"] = include_location;
        data["__TOP_MODULE_NAME__"] = dst_module_name;

        data["__XDATA_DECLARATION__"]  = xdata_declaration;
        data["__XDATA_REINIT__"]       = xdata_reinit;
        data["__XDATA_BIND__"]         = xdata_bindrw;
        data["__XPORT_ADD__"]          = xport_add;
        data["__XPORT_CASCADED_DEC__"] = cascaded_signals_dec;
        data["__XPORT_CASCADED_SGN__"] = cascaded_signals_sgn;

        data["__COMMENTS__"]         = comments;
        data["__COPY_XSPCOMM_LIB__"] = opts.cp_lib;

        // Render
        inja::Environment env;
        recursive_render(src_dir, dst_dir, data, env);
    }
} // namespace codegen
} // namespace picker
