#include <bits/stdc++.h>
#include "codegen/scala.hpp"

namespace picker { namespace codegen {

    namespace scala_ns {
        static const std::string xdata_init_template =
            "    var {{pin_uniq_name}} = new XData({{logic_pin_length}}, XData.{{logic_pin_type}})\n";
        static const std::string xdata_bindrw_template =
            "    this.{{pin_uniq_name}}.BindDPIPtr(this.dut.GetDPIHandle(\"{{pin_func_name}}\", 0), this.dut.GetDPIHandle(\"{{pin_func_name}}\", 1))\n";
        static const std::string xport_add_template =
            "    this.xport.Add(\"{{pin_func_name}}\", this.{{pin_uniq_name}})\n";
        static const std::string xport_cascaded_sgn_template =
            "    var {{port_name}} = this.xport.NewSubPort(\"{{prefix_key}}_\")\n";

        /// @brief Export external pin for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_external_pin(std::vector<picker::sv_signal_define> pin, std::string &moudle_name,
                                 std::string &xdata_init, std::string &xdata_bindrw, std::string &xport_add,
                                 std::string &swig_constant)
        {
            inja::Environment env;
            nlohmann::json data;
            auto pin_map        = picker::get_default_confilct_map();
            data["moudle_name"] = moudle_name;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = (pin[i].logic_pin_type[0] == 'i') ? "In" : "Out";
                data["pin_func_name"]  = replace_all(pin[i].logic_pin, ".", "_");
                data["pin_uniq_name"]  = picker::fix_conflict_pin_name(data["pin_func_name"], pin_map, false);

                data["logic_pin_length"] = pin[i].logic_pin_hb == -1 ? // means not vector
                                               0 :
                                               pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;

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
        void render_internal_signal(std::vector<picker::sv_signal_define> pin, std::string &moudle_name,
                                    std::string &xdata_init, std::string &xdata_bindrw, std::string &xport_add,
                                    std::string &swig_constant)
        {
            inja::Environment env;
            nlohmann::json data;
            auto pin_map        = picker::get_default_confilct_map();
            data["moudle_name"] = moudle_name;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"]  = replace_all(pin[i].logic_pin, ".", "_");
                data["pin_uniq_name"]  = picker::fix_conflict_pin_name(data["pin_func_name"], pin_map, false);

                data["logic_pin_length"] = pin[i].logic_pin_hb == -1 ? // means not vector
                                               0 :
                                               pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;
                data["logic_pin_type"] = "Out";

                xdata_init   = xdata_init + env.render(xdata_init_template, data);
                xdata_bindrw = xdata_bindrw + env.render(xdata_bindrw_template, data);
                xport_add    = xport_add + env.render(xport_add_template, data);
            }
        };

        void render_cascaded_signals(std::string prefix, nlohmann::json &signal_tree_json, std::string &cascaded_ports)
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
                    cascaded_ports += env.render(xport_cascaded_sgn_template, data);
                    render_cascaded_signals(port_name, port, cascaded_ports);
                }
            }
        }

    } // namespace scala_ns

    void scala(picker::export_opts &opts, std::vector<picker::sv_signal_define> external_pin,
               std::vector<picker::sv_signal_define> internal_signal, nlohmann::json &signal_tree_json)
    {
        std::string src_dir         = opts.source_dir + "/scala";
        std::string dst_dir         = opts.target_dir + "/scala";
        std::string dst_module_name = opts.target_module_name;
        std::string simulator       = opts.sim;

        // Codegen Buffers
        std::string xdata_init, xdata_bindrw, xport_add, swig_constant, cascaded_signals_sgn;

        // Generate External Pin
        scala_ns::render_external_pin(external_pin, dst_module_name, xdata_init, xdata_bindrw, xport_add,
                                      swig_constant);
        // Generate Internal Signal
        scala_ns::render_internal_signal(internal_signal, dst_module_name, xdata_init, xdata_bindrw, xport_add,
                                         swig_constant);
        // Generate Cascaded Porst
        scala_ns::render_cascaded_signals("", signal_tree_json, cascaded_signals_sgn);

        // Simulator
        std::transform(simulator.begin(), simulator.end(), simulator.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        // Set global render data
        nlohmann::json data;

        std::string erro_message;
        auto scala_location = picker::get_xcomm_lib("scala/xspcomm-scala.jar", erro_message);
        if (scala_location.empty()) { PK_FATAL("%s\n", erro_message.c_str()); }
        auto cpplib_location = picker::get_xcomm_lib("lib", erro_message);
        if (cpplib_location.empty()) { PK_FATAL("%s\n", erro_message.c_str()); }

        data["__XSPCOMM_LIB__"]     = cpplib_location;
        data["__XSPCOMM_JAR__"]     = scala_location;
        data["__TOP_MODULE_NAME__"] = dst_module_name;

        data["__XDATA_INIT__"]         = xdata_init;
        data["__XDATA_BIND__"]         = xdata_bindrw;
        data["__XPORT_ADD__"]          = xport_add;
        data["__XPORT_CASCADED_SGN__"] = cascaded_signals_sgn;

        data["__SWIG_CONSTANT__"]    = swig_constant;
        data["__USE_SIMULATOR__"]    = "USE_" + simulator;
        data["__COPY_XSPCOMM_LIB__"] = opts.cp_lib;

        // Render
        inja::Environment env;
        recursive_render(src_dir, dst_dir, data, env);
    }
}} // namespace picker::codegen
