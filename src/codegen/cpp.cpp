#include <bits/stdc++.h>
#include "codegen/cpp.hpp"

namespace picker {

std::string replace_all(std::string str, const std::string &from,
                        const std::string &to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos +=
            to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

namespace codegen {

    namespace cxx {
        static const std::string xdata_declaration_template =
            "    XData {{pin_func_name}};\n";
        static const std::string xdata_reinit_template =
            "    this->{{pin_func_name}}.ReInit({{logic_pin_length}}, IOType::{{logic_pin_type}}, \"{{logic_pin}}\");\n";
        static const std::string xdata_bindrw_template =
            "    this->{{pin_func_name}}.BindDPIRW({{read_func_type}}get_{{pin_func_name}}, {{write_func_type}}set_{{pin_func_name}});\n";
        static const std::string xdata_bind_onlyr_template =
            "    this->{{pin_func_name}}.BindDPIRW({{read_func_type}}get_{{pin_func_name}}, {{write_func_type}}nullptr);\n";
        static const std::string xport_add_template =
            "    this->port.Add(this->{{pin_func_name}}.mName, this->{{pin_func_name}});\n";
        static const std::string comment_template =
            "    {{logic_pin_type}} {{logic_pin_length}} {{logic_pin}}\n";

#define BIND_DPI_RW                                                            \
    if (pin[i].logic_pin_hb == -1) {                                           \
        data["logic_pin_length"] = 0;                                          \
        data["read_func_type"]   = "(void (*)(void*))";                        \
        data["write_func_type"]  = "(void (*)(const unsigned char))";          \
    } else {                                                                   \
        data["logic_pin_length"] =                                             \
            pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;                     \
        data["read_func_type"]  = "(void (*)(void*))";                         \
        data["write_func_type"] = "(void (*)(const void*))";                   \
    }

        /// @brief Export external pin for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_external_pin(std::vector<sv_signal_define> pin,
                                 std::string &xdata_declaration,
                                 std::string &xdata_reinit,
                                 std::string &xdata_bindrw,
                                 std::string &xport_add, std::string &comments)
        {
            inja::Environment env;
            nlohmann::json data;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");

                // Set 0 for 1bit singal or hb-lb+1 for vector signal for cpp
                // render
                data["logic_pin_type"] = first_upercase(pin[i].logic_pin_type);

                BIND_DPI_RW;
                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ? // means not vector
                        0 :
                        pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;

                xdata_declaration =
                    xdata_declaration
                    + env.render(xdata_declaration_template, data);
                xdata_reinit =
                    xdata_reinit + env.render(xdata_reinit_template, data);
                xdata_bindrw =
                    xdata_bindrw + env.render(xdata_bindrw_template, data);
                xport_add = xport_add + env.render(xport_add_template, data);
            }
        }

        /// @brief Export internal signal for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_internal_signal(std::vector<sv_signal_define> pin,
                                    std::string &xdata_declaration,
                                    std::string &xdata_reinit,
                                    std::string &xdata_bindrw,
                                    std::string &xport_add,
                                    std::string &comments)
        {
            inja::Environment env;
            nlohmann::json data;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");

                // Set empty or [hb:lb] for verilog render
                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ?
                        "" :
                        "[" + std::to_string(pin[i].logic_pin_hb) + ":"
                            + std::to_string(pin[i].logic_pin_lb) + "]";

                comments = comments + env.render(comment_template, data);

                // Set 0 for 1bit singal or hb-lb+1 for vector signal for cpp
                // render
                BIND_DPI_RW;
                data["logic_pin_type"] = "Output";

                xdata_declaration =
                    xdata_declaration
                    + env.render(xdata_declaration_template, data);
                xdata_reinit =
                    xdata_reinit + env.render(xdata_reinit_template, data);
                xdata_bindrw =
                    xdata_bindrw + env.render(xdata_bind_onlyr_template, data);
                xport_add = xport_add + env.render(xport_add_template, data);
            }
        };

    } // namespace cxx

    void cpp(const cxxopts::ParseResult &opts, nlohmann::json &sync_opts,
             std::vector<sv_signal_define> external_pin,
             std::vector<sv_signal_define> internal_signal)
    {
        //
        std::string src_dir = opts["source_dir"].as<std::string>() + "/cpp";
        std::string dst_dir = opts["target_dir"].as<std::string>() + "/cpp";
        std::string src_module_name = sync_opts["src_module_name"];
        std::string dst_module_name = sync_opts["dst_module_name"];

        // Codegen Buffers
        std::string pin_connect, logic, wire, comments, dpi_export, dpi_impl,
            xdata_declaration, xdata_reinit, xdata_bindrw, xport_add;

        // Generate External Pin
        cxx::render_external_pin(external_pin, xdata_declaration, xdata_reinit,
                                 xdata_bindrw, xport_add, comments);
        // Generate Internal Signal
        cxx::render_internal_signal(internal_signal, xdata_declaration,
                                    xdata_reinit, xdata_bindrw, xport_add,
                                    comments);


        // Set global render data
        nlohmann::json data;

        data["__SOURCE_MODULE_NAME__"] = sync_opts["src_module_name"];
        data["__TOP_MODULE_NAME__"]    = sync_opts["dst_module_name"];

        data["__XDATA_DECLARATION__"] = xdata_declaration;
        data["__XDATA_REINIT__"]      = xdata_reinit;
        data["__XDATA_BIND__"]        = xdata_bindrw;
        data["__XPORT_ADD__"]         = xport_add;
        data["__COMMENTS__"]          = comments;

        // Render
        inja::Environment env;
        recursive_render(src_dir, dst_dir, data, env);
    }
} // namespace codegen
} // namespace picker
