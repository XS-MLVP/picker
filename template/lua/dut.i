
%module(directors="1") _UT_{{__TOP_MODULE_NAME__}}

%feature("director") DutUnifiedBase;
%ignore DutUnifiedBase::DutUnifiedBase(std::initializer_list<const char *> args);

%define {{__USE_SIMULATOR__}}
%enddef


%apply unsigned long long {u_int64_t}
%apply unsigned int {u_uint32_t}
%apply unsigned short {u_uint16_t}
%apply unsigned char {u_uint8_t}

%apply unsigned long long {uint64_t}
%apply unsigned int {uint32_t}
%apply unsigned short {uint16_t}
%apply unsigned char {uint8_t}

%apply long long {i_int64_t}
%apply int {i_int32_t}
%apply short {i_int16_t}
%apply char {i_int8_t}

%apply long long {int64_t}
%apply int {int32_t}
%apply short {int16_t}
%apply char {int8_t}

%include std_string.i
%include std_map.i
%include std_vector.i

namespace std {
   %template(StringVector) vector<string>;
}

%{
#include "dut_base.hpp"
%}

%include "dut_base.hpp"

