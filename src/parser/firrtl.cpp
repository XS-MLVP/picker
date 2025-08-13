
#include "parser/firrtl.hpp"
#include <fstream>
#include <sstream>
#include <regex>

namespace picker { namespace parser {

    // Forward declarations
    static std::vector<std::pair<std::string, std::string>> parse_single_field(const std::string& field, const std::string& prefix);
    static std::vector<std::pair<std::string, std::string>> parse_bundle_fields(const std::string& bundle_str, const std::string& prefix);

    // Parse FIRRTL type and extract width information
    static std::pair<int, int> parse_firrtl_type(const std::string& type_str) {
        // UInt<8> -> width 8, high bit 7, low bit 0
        // SInt<16> -> width 16, high bit 15, low bit 0  
        // Clock, Reset -> width 1, high bit -1 (special)
        
        std::regex width_regex(R"((UInt|SInt)<(\d+)>)");
        std::smatch match;
        
        if (std::regex_search(type_str, match, width_regex)) {
            int width = std::stoi(match[2].str());
            return {width - 1, 0}; // high bit, low bit
        }
        
        // Handle special types
        if (type_str == "Clock" || type_str == "Reset" || 
            type_str == "UInt<1>" || type_str == "SInt<1>") {
            return {-1, -1}; // Single bit signal
        }
        
        // Default case - assume single bit
        return {-1, -1};
    }

    // Parse a single field within a bundle
    static std::vector<std::pair<std::string, std::string>> parse_single_field(const std::string& field, const std::string& prefix) {
        std::vector<std::pair<std::string, std::string>> results;
        
        // Remove @[annotation] if present
        std::string clean_field = std::regex_replace(field, std::regex(R"(\s*@\[.*?\]\s*)"), "");
        
        // Match field pattern: [flip] name : type
        std::regex field_regex(R"(\s*(flip\s+)?(\w+)\s*:\s*(.+))");
        std::smatch match;
        
        if (std::regex_search(clean_field, match, field_regex)) {
            bool is_flip = !match[1].str().empty();
            std::string field_name = match[2].str();
            std::string field_type = match[3].str();
            
            // Build full field name with prefix
            std::string full_name = prefix.empty() ? field_name : prefix + "$$" + field_name;
            
            // Check if this is a bundle type (contains braces)
            if (field_type.find('{') != std::string::npos) {
                // Recursive case: parse nested bundle
                auto nested_fields = parse_bundle_fields(field_type, full_name);
                results.insert(results.end(), nested_fields.begin(), nested_fields.end());
            } else {
                // Base case: simple type
                std::string direction = is_flip ? "input" : "output"; // flip reverses direction
                results.push_back({full_name, field_type + "|" + direction});
            }
        }
        
        return results;
    }

    // Parse bundle type and extract nested fields recursively
    static std::vector<std::pair<std::string, std::string>> parse_bundle_fields(const std::string& bundle_str, const std::string& prefix = "") {
        std::vector<std::pair<std::string, std::string>> fields;
        
        // Remove outer braces
        std::string content = bundle_str;
        if (content.front() == '{' && content.back() == '}') {
            content = content.substr(1, content.length() - 2);
        }
        
        // Parse fields separated by commas, handling nested braces
        size_t pos = 0;
        int brace_depth = 0;
        size_t field_start = 0;
        
        while (pos < content.length()) {
            char c = content[pos];
            
            if (c == '{') {
                brace_depth++;
            } else if (c == '}') {
                brace_depth--;
            } else if (c == ',' && brace_depth == 0) {
                // Found field separator at top level
                std::string field = content.substr(field_start, pos - field_start);
                field = std::regex_replace(field, std::regex(R"(^\s+|\s+$)"), ""); // trim
                
                if (!field.empty()) {
                    auto parsed_fields = parse_single_field(field, prefix);
                    fields.insert(fields.end(), parsed_fields.begin(), parsed_fields.end());
                }
                
                field_start = pos + 1;
            }
            pos++;
        }
        
        // Handle last field
        if (field_start < content.length()) {
            std::string field = content.substr(field_start);
            field = std::regex_replace(field, std::regex(R"(^\s+|\s+$)"), ""); // trim
            
            if (!field.empty()) {
                auto parsed_fields = parse_single_field(field, prefix);
                fields.insert(fields.end(), parsed_fields.begin(), parsed_fields.end());
            }
        }
        
        return fields;
    }

    // Parse FIRRTL port lines (handles both simple and bundle types)
    static std::vector<sv_signal_define> parse_firrtl_port(const std::string& port_line) {
        std::vector<sv_signal_define> pins;
        
        // Format: "input clock : Clock" or "output result : UInt<32>" 
        // or "output difftest : { step : UInt<64>, ... }"
        std::regex port_regex(R"(\s*(input|output)\s+(\w+)\s*:\s*(.+))");
        std::smatch match;
        
        if (std::regex_search(port_line, match, port_regex)) {
            std::string port_direction = match[1].str(); // "input" or "output"
            std::string port_name = match[2].str();      // port name
            std::string type_str = match[3].str();       // type string
            
            // Remove @[annotation] if present
            type_str = std::regex_replace(type_str, std::regex(R"(\s*@\[.*?\]\s*)"), "");
            
            // Check if this is a bundle type
            if (type_str.find('{') != std::string::npos) {
                // Parse bundle fields
                auto fields = parse_bundle_fields(type_str, port_name);
                
                for (const auto& [field_name, field_info] : fields) {
                    sv_signal_define pin;
                    pin.logic_pin = field_name;
                    
                    // Parse field_info: "type|direction"
                    size_t sep_pos = field_info.find('|');
                    if (sep_pos != std::string::npos) {
                        std::string field_type = field_info.substr(0, sep_pos);
                        std::string field_direction = field_info.substr(sep_pos + 1);
                        
                        // For bundle fields, flip reverses the port direction
                        if (field_direction == "input") {
                            pin.logic_pin_type = (port_direction == "output") ? "input" : "output";
                        } else {
                            pin.logic_pin_type = port_direction;
                        }
                        
                        auto [high_bit, low_bit] = parse_firrtl_type(field_type);
                        pin.logic_pin_hb = high_bit;
                        pin.logic_pin_lb = low_bit;
                        
                        pins.push_back(pin);
                        
                        PK_DEBUG("Parsed bundle field: %s %s : %s (bits [%d:%d])", 
                                 pin.logic_pin_type.c_str(), pin.logic_pin.c_str(), 
                                 field_type.c_str(), pin.logic_pin_hb, pin.logic_pin_lb);
                    }
                }
            } else {
                // Simple type
                sv_signal_define pin;
                pin.logic_pin_type = port_direction;
                pin.logic_pin = port_name;
                
                auto [high_bit, low_bit] = parse_firrtl_type(type_str);
                pin.logic_pin_hb = high_bit;
                pin.logic_pin_lb = low_bit;
                
                pins.push_back(pin);
                
                PK_DEBUG("Parsed simple port: %s %s : %s (bits [%d:%d])", 
                         pin.logic_pin_type.c_str(), pin.logic_pin.c_str(), 
                         type_str.c_str(), pin.logic_pin_hb, pin.logic_pin_lb);
            }
        }
        
        return pins;
    }

    // Parse FIRRTL file and extract main circuit module and its ports
    int firrtl(picker::export_opts &opts, std::vector<picker::sv_module_define> &external_pin){
        if(opts.rw_type != picker::SignalAccessType::MEM_DIRECT) {
                PK_FATAL("GSIM only support MEM_DIRECT access mode, please use --rw mem_direct");
        }
        if(opts.file.empty()) {
            PK_FATAL("No input file given for FIRRTL parser. Please specify a .fir file.");
        }
        if(opts.file.size() > 1) {
            PK_FATAL("FIRRTL parser currently supports only one input file. Multiple files provided.");
        }
        
        auto firrtl_file = opts.file[0];
        if(firrtl_file.substr(firrtl_file.find_last_of(".") + 1) != "fir") {
            PK_FATAL("Invalid file type for FIRRTL parser. Expected .fir file, got: %s", firrtl_file.c_str());
        }
        
        PK_MESSAGE("Parsing FIRRTL file: %s", firrtl_file.c_str());
        
        // Read FIRRTL file
        std::ifstream file(firrtl_file);
        if (!file.is_open()) {
            PK_FATAL("Cannot open FIRRTL file: %s", firrtl_file.c_str());
        }
        
        std::string line;
        std::string circuit_name;
        std::string main_module_name;
        std::vector<sv_signal_define> ports;
        
        bool in_main_module = false;
        bool parsing_ports = false;
        
        // Parse FIRRTL file line by line
        while (std::getline(file, line)) {
            // Remove leading/trailing whitespace
            line = std::regex_replace(line, std::regex(R"(^\s+|\s+$)"), "");
            
            if (line.empty() || line[0] == ';') continue; // Skip empty lines and comments
            
            // Parse circuit declaration: "circuit MyCircuit :"
            std::regex circuit_regex(R"(circuit\s+(\w+)\s*:)");
            std::smatch circuit_match;
            if (std::regex_search(line, circuit_match, circuit_regex)) {
                circuit_name = circuit_match[1].str();
                main_module_name = circuit_name; // Main module typically has same name as circuit
                PK_MESSAGE("Found circuit: %s", circuit_name.c_str());
                continue;
            }
            
            // Parse module declaration: "module MyModule :"
            std::regex module_regex(R"(module\s+(\w+)\s*:)");
            std::smatch module_match;
            if (std::regex_search(line, module_match, module_regex)) {
                std::string module_name = module_match[1].str();
                
                // Check if this is the main module (same name as circuit)
                if (module_name == circuit_name) {
                    in_main_module = true;
                    parsing_ports = true;
                    PK_MESSAGE("Found main module: %s", module_name.c_str());
                } else {
                    in_main_module = false;
                    parsing_ports = false;
                }
                continue;
            }
            
            // Parse ports in main module
            if (in_main_module && parsing_ports) {
                // Check for port lines: "input clock : Clock" or "output result : UInt<32>"
                if (line.find("input ") == 0 || line.find("output ") == 0) {
                    std::vector<sv_signal_define> parsed_ports = parse_firrtl_port(line);
                    for (const auto& port : parsed_ports) {
                        if (!port.logic_pin.empty()) {
                            ports.push_back(port);
                        }
                    }
                }
                // Stop parsing ports when we hit non-port content
                else if (!line.empty() && line.find("input ") != 0 && line.find("output ") != 0) {
                    parsing_ports = false;
                }
            }
        }
        
        file.close();
        
        if (circuit_name.empty()) {
            PK_FATAL("No circuit declaration found in FIRRTL file: %s", firrtl_file.c_str());
        }
        
        if (ports.empty()) {
            PK_MESSAGE("Warning: No ports found in main module %s", main_module_name.c_str());
        }
        
        // Create module definition
        sv_module_define main_module;
        main_module.module_name = main_module_name;
        main_module.module_nums = 1;
        main_module.pins = ports;
        
        external_pin.push_back(main_module);
        
        // Set target module name
        if (opts.target_module_name.empty()) {
            opts.target_module_name = main_module_name;
        }
        
        // Set target directory
        if (opts.target_dir.empty() || opts.target_dir.back() == '/') {
            opts.target_dir += opts.target_module_name;
            PK_DEBUG("Set target dir to target module name: %s", opts.target_dir.c_str());
        }
        
        // Debug output
        PK_MESSAGE("Successfully parsed FIRRTL circuit: %s", circuit_name.c_str());
        PK_DEBUG("Main module: %s with %d ports", main_module_name.c_str(), (int)ports.size());
        for (const auto& port : ports) {
            PK_DEBUG("Port: %s %s [%d:%d]", port.logic_pin_type.c_str(), 
                     port.logic_pin.c_str(), port.logic_pin_hb, port.logic_pin_lb);
        }
        
        return 0;
    }
}}
