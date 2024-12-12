#pragma once

#include <bits/stdc++.h>
#include "picker.hpp"

namespace picker { namespace parser {
    int sv(picker::export_opts &opts, std::vector<picker::sv_module_define> &external_pin);
    int uvm(picker::pack_opts &opts, std::string &path, std::string &filename, uvm_transaction_define &uvm_transaction);
}; } // namespace picker::parser