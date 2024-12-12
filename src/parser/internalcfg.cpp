#include "parser/internalcfg.hpp"

namespace picker { namespace parser {

    void recursive_parse(YAML::Node node, std::vector<picker::sv_signal_define> &pin_list, std::string prefix)
    {
        if (node.IsMap()) {
            for (auto it = node.begin(); it != node.end(); ++it) {
                std::string key = it->first.as<std::string>();
                if (prefix.length() == 0)
                    recursive_parse(it->second, pin_list, key);
                else
                    recursive_parse(it->second, pin_list, prefix + "." + key);
            }
        } else if (node.IsSequence()) {
            for (auto it = node.begin(); it != node.end(); ++it) { recursive_parse(*it, pin_list, prefix); }
        } else if (node.IsScalar()) {
            sv_signal_define pin;
            std::string value = node.as<std::string>(); // such as "input [31:0] Signalname"
            std::vector<std::string> tokens;
            std::string token;
            std::istringstream tokenStream(value);
            while (std::getline(tokenStream, token, ' ')) { tokens.push_back(token); }
            if (tokens.size() == 2) {
                pin.logic_pin      = prefix + "." + tokens[1];
                pin.logic_pin_type = tokens[0];
                pin.logic_pin_hb   = -1;
            } else if (tokens.size() == 3) {
                pin.logic_pin      = prefix + "." + tokens[2];
                pin.logic_pin_type = tokens[0];
                int pos            = tokens[1].find(":");
                pin.logic_pin_hb   = std::stoi(tokens[1].substr(1, pos - 1));
                pin.logic_pin_lb   = std::stoi(tokens[1].substr(pos + 1, tokens[1].length() - pos - 2));

            } else {
                PK_MESSAGE("error: unknown node type");
            }
            pin_list.push_back(pin);
        } else {
            PK_MESSAGE("error: unknown node type");
        }
    }

    std::vector<picker::sv_signal_define> internal(std::string internal_pin_filename)
    {
        std::vector<picker::sv_signal_define> pin_list;
        YAML::Node internal_pin_node = YAML::LoadFile(internal_pin_filename);
        recursive_parse(internal_pin_node, pin_list);
        return pin_list;
    };

    int internal(picker::export_opts &opts, std::vector<picker::sv_signal_define> &internal_pin)
    {
        internal_pin = opts.internal.length() == 0 ? std::vector<picker::sv_signal_define>() : internal(opts.internal);
        return 0;
    };

}} // namespace picker::parser