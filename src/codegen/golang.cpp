#include <bits/stdc++.h>
#include "codegen/golang.hpp"

namespace picker { namespace codegen {

    namespace go {
        static const std::string xdata_declare_template =
            "    P_{{pin_func_name}} xspcomm.XData\n";
        static const std::string xdata_init_template =
            "    self.P_{{pin_func_name}} = xspcomm.NewXData({{logic_pin_length}}, xspcomm.{{logic_pin_type}})\n";
        static const std::string xdata_bindrw_template =
            "    self.P_{{pin_func_name}}.BindDPIPtr(self.dut.GetDPIHandle(\"{{pin_func_name}}\", 0), self.dut.GetDPIHandle(\"{{pin_func_name}}\", 1))\n";
        static const std::string xport_add_template =
            "    self.Port.Add(\"{{pin_func_name}}\", self.P_{{pin_func_name}})\n";

        /// @brief Export external pin for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_external_pin(std::vector<picker::sv_signal_define> pin,
                                 std::string &xdata_decl,
                                 std::string &xdata_init, 
                                 std::string &xdata_bindrw,
                                 std::string &xport_add,
                                 std::string &swig_constant)
        {
            inja::Environment env;
            nlohmann::json data;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"] = pin[i].logic_pin;
                data["logic_pin_type"] =
                    (pin[i].logic_pin_type[0] == 'i') ? "IOType_Input" : "IOType_Output";
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");

                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ? // means not vector
                        0 :
                        pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;

                xdata_decl =
                    xdata_decl + env.render(xdata_declare_template, data);

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
                                    std::string &xdata_decl,
                                    std::string &xdata_init,
                                    std::string &xdata_bindrw,
                                    std::string &xport_add,
                                    std::string &swig_constant)
        {
            inja::Environment env;
            nlohmann::json data;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"] = replace_all(pin[i].logic_pin, ".", "_");

                data["logic_pin_length"] =
                    pin[i].logic_pin_hb == -1 ? // means not vector
                        0 :
                        pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;
                data["logic_pin_type"] = "Out";
                
                xdata_decl =
                    xdata_decl + env.render(xdata_declare_template, data);

                xdata_init = xdata_init + env.render(xdata_init_template, data);
                xdata_bindrw =
                    xdata_bindrw + env.render(xdata_bindrw_template, data);
                xport_add = xport_add + env.render(xport_add_template, data);
            }
        };

    } // namespace golang

    void golang(picker::export_opts &opts,
                std::vector<picker::sv_signal_define> external_pin,
                std::vector<picker::sv_signal_define> internal_signal)
    {
        //
        std::string src_dir         = opts.source_dir + "/golang";
        std::string dst_dir         = opts.target_dir + "/golang";
        std::string src_module_name = opts.source_module_name;
        std::string dst_module_name = opts.target_module_name;
        std::string simulator       = opts.sim;

        // Codegen Buffers
        std::string xdata_decl, xdata_init, xdata_bindrw, xport_add, swig_constant;

        // Generate External Pin
        go::render_external_pin(external_pin, xdata_decl, xdata_init, xdata_bindrw,
                                xport_add, swig_constant);
        // Generate Internal Signal
        go::render_internal_signal(internal_signal, xdata_decl, xdata_init, xdata_bindrw,
                                   xport_add, swig_constant);

        // Simulator
        std::transform(simulator.begin(), simulator.end(), simulator.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        // Set global render data
        nlohmann::json data;

        std::string erro_message;
        auto golang_location =
            picker::get_xcomm_lib("golang", erro_message);
        if (golang_location.empty()) { FATAL("%s\n", erro_message.c_str()); }
        data["__XSPCOMM_GOLANG__"] = golang_location;
        data["__XDATA_DECL__"]    = xdata_decl;

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
