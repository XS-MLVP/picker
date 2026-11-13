#pragma once

#include <string>

namespace picker::codegen::sv::templates {

// Shared module, port and DPI fragments. Simulator waveform/coverage fragments
// stay with their renderer in src/codegen/sv/waveform.cpp.
inline const std::string sub_module = " {{module_name}} {{module_name}}{{module_subfix}}(\n{{pin_connect}}\n );\n";
inline const std::string pin_connect = "    .{{raw_pin}}({{logic_pin}}),\n";
inline const std::string logic_pin_declaration =
    "  {{logic_pin_data_type}} {{logic_pin_length}} {{logic_pin}}{{unpacked_dimensions}};\n";
inline const std::string wire_pin_declaration =
    "  {{wire_pin_data_type}} {{logic_pin_length}} {{logic_pin}}{{unpacked_dimensions}};\n";

inline const std::string dpi_get_export =
    "  export \"DPI-C\" function get_{{pin_func_name}}xx{{__LIB_DPI_FUNC_NAME_HASH__}};\n";
inline const std::string dpi_set_export =
    "  export \"DPI-C\" function set_{{pin_func_name}}xx{{__LIB_DPI_FUNC_NAME_HASH__}};\n";

inline const std::string dpi_get_impl =
    "  function void get_{{pin_func_name}}xx{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    output logic {{logic_pin_length}} value;\n"
    "    value={{logic_pin}};\n"
    "  endfunction\n\n";

inline const std::string dpi_set_impl =
    "  function void set_{{pin_func_name}}xx{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    input logic {{logic_pin_length}} value;\n"
    "    {{logic_pin}}=value;\n"
    "  endfunction\n\n";

inline const std::string dpi_array_get_impl =
    "  function void get_{{pin_func_name}}xx{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    input int index;\n"
    "    output logic {{logic_pin_length}} value;\n"
    "    if (index < 0 || index >= {{array_size}}) value='x;\n"
    "    else value={{logic_pin}};\n"
    "  endfunction\n\n";

inline const std::string dpi_array_set_impl =
    "  function void set_{{pin_func_name}}xx{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    input int index;\n"
    "    input logic {{logic_pin_length}} value;\n"
    "    if (index >= 0 && index < {{array_size}}) {{logic_pin}}=value;\n"
    "  endfunction\n\n";

inline const std::string dpi_finish =
    "  export \"DPI-C\" function finish_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "  function void finish_{{__LIB_DPI_FUNC_NAME_HASH__}};\n"
    "    $finish;\n"
    "  endfunction\n";

} // namespace picker::codegen::sv::templates
