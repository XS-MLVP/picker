#pragma once

#include "picker.hpp"

namespace picker { namespace parser {

    void parse_sv_transactions_with_slang(const picker::pack_opts &opts,
                                          std::vector<picker::uvm_transaction_define> &transactions);

}} // namespace picker::parser
