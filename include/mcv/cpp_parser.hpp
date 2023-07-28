#ifndef __MCV_CPP_PARSER__
#define __MCV_CPP_PARSER__

#include <cppast/parser.hpp>

namespace MCV
{
    class CppParser : public cppast::parser
    {
        cppast::stderr_diagnostic_logger logger;
        CppParser() : cppast::parser(type_safe::ref(CppParser::logger)) {}
    };
}
#endif