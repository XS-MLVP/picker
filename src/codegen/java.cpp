#include "codegen/names.hpp"
#include "codegen/render_tags.hpp"
#include "codegen/java.hpp"
#include "codegen/lib.hpp"

namespace picker { namespace codegen {

    namespace java_ns {
        static const std::string xdata_declare_template = "    public XData {{pin_uniq_name}};\n";
        static const std::string xdata_init_template =
            "        this.{{pin_uniq_name}} = new XData({{logic_pin_length}}, XData.{{logic_pin_type}});\n";
        static const std::string xdata_binddpi_template =
            "        this.{{pin_uniq_name}}.BindDPIPtr(this.dut.GetDPIHandle(\"{{pin_func_name}}\", 0), this.dut.GetDPIHandle(\"{{pin_func_name}}\", 1));\n";
        static const std::string xdata_bindptr_template =
            "        this.{{pin_uniq_name}}.BindNativeData(this.dut.NativeSignalAddr(\"{{logic_pin}}\"));\n";
        static const std::string xport_add_template =
            "        this.xport.Add(\"{{signal_name}}\", this.{{pin_uniq_name}});\n";
        static const std::string xport_cascaded_dec_template = "    public XPort {{port_name}};\n";
        static const std::string xport_cascaded_sgn_template =
            "        this.{{port_name}} = this.xport.NewSubPort(\"{{prefix_key}}_\");\n";

        /// @brief Export external pin for cpp render
        /// @param pin
        /// @param xdata_declaration
        /// @param xdata_reinit
        /// @param xdata_bindrw
        /// @param xport_add
        /// @param comments
        void render_external_pin(std::vector<picker::sv_signal_define> pin, std::string &moudle_name,
                                  std::string &xdata_decl, std::string &xdata_init, std::string &xdata_bindrw,
                                  std::string &xport_add, SignalAccessType rw_type, const MemberNames &names)
        {
            inja::Environment env;
            nlohmann::json data;
            data["moudle_name"] = moudle_name;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin_direction(pin[i].logic_pin_type);
                data["pin_func_name"]  = str_replace_all(pin[i].logic_pin, ".", "_");
                data["signal_name"]    = signal_name(pin[i]);
                data["pin_uniq_name"]  = names.Pin(pin[i].logic_pin);

                data["logic_pin_length"] = pin[i].logic_pin_hb == -1 ? // means not vector
                                               0 :
                                               pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;

                xdata_decl   = xdata_decl + env.render(xdata_declare_template, data);
                xdata_init   = xdata_init + env.render(xdata_init_template, data);
                xdata_bindrw += !pin[i].array_name.empty() ?
                    render_array_binding(pin[i], "java", data["pin_uniq_name"]) :
                    env.render(rw_type == SignalAccessType::MEM_DIRECT ? xdata_bindptr_template : xdata_binddpi_template, data);
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
                                    std::string &xdata_decl, std::string &xdata_init, std::string &xdata_bindrw,
                                    std::string &xport_add, const MemberNames &names)
        {
            inja::Environment env;
            nlohmann::json data;
            data["moudle_name"] = moudle_name;
            for (int i = 0; i < pin.size(); i++) {
                data["logic_pin"]      = pin[i].logic_pin;
                data["logic_pin_type"] = pin[i].logic_pin_type;
                data["pin_func_name"]  = str_replace_all(pin[i].logic_pin, ".", "_");
                data["signal_name"]    = data["pin_func_name"];
                data["pin_uniq_name"]  = names.Pin(pin[i].logic_pin);

                data["logic_pin_length"] = pin[i].logic_pin_hb == -1 ? // means not vector
                                               0 :
                                               pin[i].logic_pin_hb - pin[i].logic_pin_lb + 1;
                data["logic_pin_type"] = "Out";

                xdata_decl   = xdata_decl + env.render(xdata_declare_template, data);
                xdata_init   = xdata_init + env.render(xdata_init_template, data);
                xdata_bindrw = xdata_bindrw + env.render(xdata_binddpi_template, data);
                xport_add    = xport_add + env.render(xport_add_template, data);
            }
        };

        void render_cascaded_signals(std::string prefix, nlohmann::json &signal_tree_json,
                                     std::string &cascaded_ports_dec, std::string &cascaded_ports_sgn,
                                     const MemberNames &names)
        {
            nlohmann::json data;
            inja::Environment env;
            for (auto &[key, port] : signal_tree_json.items()) {
                if (port.contains("_")) { // ignore leaf node
                    continue;
                } else {
                    std::string port_name = prefix.empty() ? key : prefix + "_" + key;
                    data["port_name"]     = names.Cascade(port_name);
                    data["prefix_key"]    = port_name;
                    cascaded_ports_dec += env.render(xport_cascaded_dec_template, data);
                    cascaded_ports_sgn += env.render(xport_cascaded_sgn_template, data);
                    render_cascaded_signals(port_name, port, cascaded_ports_dec, cascaded_ports_sgn, names);
                }
            }
        }

    } // namespace java_ns

    void java(picker::export_opts &opts, std::vector<picker::sv_signal_define> external_pin,
              std::vector<picker::sv_signal_define> internal_signal, nlohmann::json &signal_tree_json)
    {
        std::string src_dir         = opts.source_dir + "/java";
        std::string dst_dir         = opts.target_dir + "/java";
        std::string dst_module_name = opts.target_module_name;
        std::string simulator       = opts.sim;

        // Codegen Buffers
        std::string xdata_decl, xdata_init, xdata_bindrw, xport_add, cascaded_signals_dec,
            cascaded_signals_sgn;

        // Generate External Pin
        const MemberNames names(external_pin, internal_signal, signal_tree_json, "java", dst_module_name);
        java_ns::render_external_pin(external_pin, dst_module_name, xdata_decl, xdata_init, xdata_bindrw, xport_add,
                                      opts.rw_type, names);
        // Generate Internal Signal
        java_ns::render_internal_signal(internal_signal, dst_module_name, xdata_decl, xdata_init, xdata_bindrw,
                                        xport_add, names);

        // Generate Cascaded Ports
        java_ns::render_cascaded_signals("", signal_tree_json, cascaded_signals_dec, cascaded_signals_sgn, names);
        render_array_ports(external_pin, "java", cascaded_signals_dec, cascaded_signals_sgn, &names);

        // Simulator
        std::transform(simulator.begin(), simulator.end(), simulator.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        // Set global render data
        nlohmann::json data;

        std::string erro_message;
        auto java_location = picker::get_xcomm_lib("java/xspcomm-java.jar", erro_message);
        if (java_location.empty()) { PK_FATAL("%s\n", erro_message.c_str()); }
        auto cpplib_location = picker::get_xcomm_lib("lib", erro_message);
        if (cpplib_location.empty()) { PK_FATAL("%s\n", erro_message.c_str()); }

        data[tags::xspcomm_lib]     = cpplib_location;
        data[tags::xspcomm_jar]     = java_location;
        data[tags::top_module_name] = dst_module_name;
        std::string native_lib_suffix = get_shared_lib_suffix();
        // The suffix of Java native library in macOS is `jnilib`
        if (native_lib_suffix == "dylib") { native_lib_suffix = "jnilib"; }
        data[tags::shared_lib_suffix] = get_shared_lib_suffix();
        data[tags::jni_lib_suffix]    = native_lib_suffix;

        data[tags::xdata_declaration]  = xdata_decl;
        data[tags::xdata_init]         = xdata_init;
        data[tags::xdata_bind]         = xdata_bindrw;
        data[tags::xport_add]          = xport_add;
        data[tags::xport_cascaded_dec] = cascaded_signals_dec;
        data[tags::xport_cascaded_sgn] = cascaded_signals_sgn;

        data[tags::copy_xspcomm_lib] = opts.cp_lib;
        data[tags::simulator]        = opts.sim;

        // Render
        configure_array_bindings(external_pin, opts, data);
        inja::Environment env;
        recursive_render(src_dir, dst_dir, data, env);
    }
}} // namespace picker::codegen
