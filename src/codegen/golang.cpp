#include <bits/stdc++.h>
#include "codegen/golang.hpp"

namespace picker { namespace codegen {

    namespace go {
        static const std::string xdata_declare_template = "    {{pin_uniq_name}} xspcomm.XData\n";
        static const std::string xdata_init_template =
            "    self.{{pin_uniq_name}} = xspcomm.NewXData({{logic_pin_length}}, xspcomm.{{logic_pin_type}})\n";
        static const std::string xdata_bindrw_template =
            "    self.{{pin_uniq_name}}.BindDPIPtr(self.Dut.GetDPIHandle(\"{{pin_func_name}}\", 0), self.Dut.GetDPIHandle(\"{{pin_func_name}}\", 1))\n";
        static const std::string xport_add_template =
            "    self.Xport.Add(\"{{pin_func_name}}\", self.{{pin_uniq_name}})\n";
        static const std::string xport_cascaded_dec_template = "    {{port_name}} xspcomm.XPort\n";
        static const std::string xport_cascaded_sgn_template =
            "    self.{{port_name}} = self.Xport.NewSubPort(\"{{prefix_key}}_\")\n";

        /// @brief Export external pin for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_external_pin(std::vector<picker::sv_signal_define> pin, std::string &xdata_decl,
                                 std::string &xdata_init, std::string &xdata_bindrw, std::string &xport_add,
                                 std::string &swig_constant)
        {
            inja::Environment env;
            nlohmann::json data;
            auto pin_map = picker::get_default_confilct_map();
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = (pin[i].logic_pin_type[0] == 'i') ? "IOType_Input" : "IOType_Output";
                data["pin_func_name"]  = replace_all(pin[i].logic_pin, ".", "_");
                data["pin_uniq_name"]  = picker::fix_conflict_pin_name(data["pin_func_name"], pin_map, true);

                data["logic_pin_length"] = pin[i].logic_pin_hb == -1 ? // means not vector
                                               0 :
                                               pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;

                xdata_decl = xdata_decl + env.render(xdata_declare_template, data);

                xdata_init   = xdata_init + env.render(xdata_init_template, data);
                xdata_bindrw = xdata_bindrw + env.render(xdata_bindrw_template, data);
                xport_add    = xport_add + env.render(xport_add_template, data);
            }
        }

        /// @brief Export internal signal for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_internal_signal(std::vector<picker::sv_signal_define> pin, std::string &xdata_decl,
                                    std::string &xdata_init, std::string &xdata_bindrw, std::string &xport_add,
                                    std::string &swig_constant)
        {
            inja::Environment env;
            nlohmann::json data;
            auto pin_map = picker::get_default_confilct_map();
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"]  = replace_all(pin[i].logic_pin, ".", "_");
                data["pin_uniq_name"]  = picker::fix_conflict_pin_name(data["pin_func_name"], pin_map, true);

                data["logic_pin_length"] = pin[i].logic_pin_hb == -1 ? // means not vector
                                               0 :
                                               pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;
                data["logic_pin_type"] = "Out";

                xdata_decl = xdata_decl + env.render(xdata_declare_template, data);

                xdata_init   = xdata_init + env.render(xdata_init_template, data);
                xdata_bindrw = xdata_bindrw + env.render(xdata_bindrw_template, data);
                xport_add    = xport_add + env.render(xport_add_template, data);
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
                    std::string port_name = picker::capitalize_first_letter(prefix.empty() ? key : prefix + "_" + key);
                    data["port_name"]     = port_name;
                    data["prefix_key"]    = port_name;
                    cascaded_ports_dec += env.render(xport_cascaded_dec_template, data);
                    cascaded_ports_sgn += env.render(xport_cascaded_sgn_template, data);
                    render_cascaded_signals(port_name, port, cascaded_ports_dec, cascaded_ports_sgn);
                }
            }
        }

    } // namespace go

    void golang(picker::export_opts &opts, std::vector<picker::sv_signal_define> external_pin,
                std::vector<picker::sv_signal_define> internal_signal, nlohmann::json &signal_tree_json)
    {
        //
        std::string src_dir         = opts.source_dir + "/golang";
        std::string dst_dir         = opts.target_dir + "/golang";
        std::string dst_module_name = opts.target_module_name;
        std::string simulator       = opts.sim;

        // Codegen Buffers
        std::string xdata_decl, xdata_init, xdata_bindrw, xport_add, swig_constant, cascaded_signals_dec,
            cascaded_signals_sgn;

        // Generate External Pin
        go::render_external_pin(external_pin, xdata_decl, xdata_init, xdata_bindrw, xport_add, swig_constant);
        // Generate Internal Signal
        go::render_internal_signal(internal_signal, xdata_decl, xdata_init, xdata_bindrw, xport_add, swig_constant);

        // Generate Cascaded Ports
        go::render_cascaded_signals("", signal_tree_json, cascaded_signals_dec, cascaded_signals_sgn);

        // Simulator
        std::transform(simulator.begin(), simulator.end(), simulator.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        // Set global render data
        nlohmann::json data;

        std::string erro_message;
        auto golang_location = picker::get_xcomm_lib("golang", erro_message);
        if (golang_location.empty()) { PK_FATAL("%s\n", erro_message.c_str()); }
        auto cpplib_location = picker::get_xcomm_lib("lib", erro_message);
        if (cpplib_location.empty()) { PK_FATAL("%s\n", erro_message.c_str()); }

        data["__XSPCOMM_LIB__"]     = cpplib_location;
        data["__XSPCOMM_GOLANG__"]  = golang_location;
        data["__XDATA_DECL__"]      = xdata_decl;
        data["__TOP_MODULE_NAME__"] = dst_module_name;

        data["__XDATA_INIT__"]         = xdata_init;
        data["__XDATA_BIND__"]         = xdata_bindrw;
        data["__XPORT_ADD__"]          = xport_add;
        data["__XPORT_CASCADED_DEC__"] = cascaded_signals_dec;
        data["__XPORT_CASCADED_SGN__"] = cascaded_signals_sgn;

        data["__SWIG_CONSTANT__"]    = swig_constant;
        data["__USE_SIMULATOR__"]    = "USE_" + simulator;
        data["__COPY_XSPCOMM_LIB__"] = opts.cp_lib;

        // Render
        inja::Environment env;
        recursive_render(src_dir, dst_dir, data, env);
    }
}} // namespace picker::codegen
