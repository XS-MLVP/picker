#include <bits/stdc++.h>
#include "codegen/python.hpp"

namespace picker { namespace codegen {

    namespace py {
        static const std::string xdata_init_template =
            "        self.{{pin_uniq_name}} = xsp.XPin(xsp.XData({{logic_pin_length}}, xsp.XData.{{logic_pin_type}}), self.event)\n";
        static const std::string xdata_bindrw_template =
            "        self.{{pin_uniq_name}}.BindDPIPtr(self.dut.GetDPIHandle(\"{{pin_func_name}}\", 0), self.dut.GetDPIHandle(\"{{pin_func_name}}\", 1))\n";
        static const std::string xport_add_template =
            "        self.xport.Add(\"{{pin_func_name}}\", self.{{pin_uniq_name}}.xdata)\n";

        /// @brief Export external pin for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_external_pin(std::vector<picker::sv_signal_define> pin,
                                 std::string &xdata_init,
                                 std::string &xdata_bindrw,
                                 std::string &xport_add,
                                 std::string &swig_constant)
        {
            inja::Environment env;
            nlohmann::json data;
            auto pin_map = picker::get_default_confilct_map();
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"] = pin[i].logic_pin;
                data["logic_pin_type"] =
                    (pin[i].logic_pin_type[0] == 'i') ? "In" : "Out";
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");
                data["pin_uniq_name"] = picker::fix_conflict_pin_name(data["pin_func_name"], pin_map, false);

                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ? // means not vector
                        0 :
                        pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;

                xdata_init = xdata_init + env.render(xdata_init_template, data);
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
        void render_internal_signal(std::vector<picker::sv_signal_define> pin,
                                    std::string &xdata_init,
                                    std::string &xdata_bindrw,
                                    std::string &xport_add,
                                    std::string &swig_constant)
        {
            inja::Environment env;
            nlohmann::json data;
            auto pin_map = picker::get_default_confilct_map();
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");
                data["pin_uniq_name"] = picker::fix_conflict_pin_name(data["pin_func_name"], pin_map, false);

                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ? // means not vector
                        0 :
                        pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;
                data["logic_pin_type"] = "Out";

                xdata_init = xdata_init + env.render(xdata_init_template, data);
                xdata_bindrw =
                    xdata_bindrw + env.render(xdata_bindrw_template, data);
                xport_add = xport_add + env.render(xport_add_template, data);
            }
        };

    } // namespace py

    void python(picker::export_opts &opts,
                std::vector<picker::sv_signal_define> external_pin,
                std::vector<picker::sv_signal_define> internal_signal)
    {
        //
        std::string src_dir         = opts.source_dir + "/python";
        std::string dst_dir         = opts.target_dir + "/python";
        std::string src_module_name = opts.source_module_name;
        std::string dst_module_name = opts.target_module_name;
        std::string simulator       = opts.sim;

        // Codegen Buffers
        std::string xdata_init, xdata_bindrw, xport_add, swig_constant;

        // Generate External Pin
        py::render_external_pin(external_pin, xdata_init, xdata_bindrw,
                                xport_add, swig_constant);
        // Generate Internal Signal
        py::render_internal_signal(internal_signal, xdata_init, xdata_bindrw,
                                   xport_add, swig_constant);

        // Simulator
        std::transform(simulator.begin(), simulator.end(), simulator.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        // Set global render data
        nlohmann::json data;

        std::string erro_message;
        auto python_location =
            picker::get_xcomm_lib("python/xspcomm", erro_message);
        if (python_location.empty()) { FATAL("%s\n", erro_message.c_str()); }
        data["__XSPCOMM_PYTHON__"] = python_location;

        data["__SOURCE_MODULE_NAME__"] = src_module_name;
        data["__TOP_MODULE_NAME__"]    = dst_module_name;

        data["__XDATA_INIT__"]    = xdata_init;
        data["__XDATA_BIND__"]    = xdata_bindrw;
        data["__XPORT_ADD__"]     = xport_add;
        data["__SWIG_CONSTANT__"] = swig_constant;
        data["__USE_SIMULATOR__"] = "USE_" + simulator;

        // Render
        inja::Environment env;
        recursive_render(src_dir, dst_dir, data, env);
    }
}} // namespace picker::codegen
