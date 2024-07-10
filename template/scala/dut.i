
%module(directors="1") libUT_{{__TOP_MODULE_NAME__}}

%include std_string.i
%include std_map.i
%include std_vector.i

%typemap(jstype) std::vector<std::string> "java.util.List<String>"
%typemap(javain) std::vector<std::string> "$javacode"
%typemap(jni) std::vector<std::string> "jobjectArray"
%template(StringVector) std::vector<std::string>;

%{
#include "dut_base.hpp"
#include "UT_{{__TOP_MODULE_NAME__}}_dpi.hpp"
%}

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

%include "dut_base.hpp"
%include "UT_{{__TOP_MODULE_NAME__}}_dpi.hpp"

%extend DutUnifiedBase {
    DutUnifiedBase(std::vector<std::string> &args) {
        int argc    = args.size();
        // Reserve heap space for VCS overflow ?
        char **argv = (char **)malloc(sizeof(char *) * (argc+128));
        memset(argv, -1, sizeof(char *) * (argc+128));
        int i       = 0;
        for (auto narg : args) {
            auto arg = narg.c_str();
            char *name = (char *)malloc(strlen(arg) + 1);
            strcpy(name, arg);
            argv[i++] = name;
        }
        return new DutUnifiedBase(argc, argv);
    }
}


%{
// Default null DPI read and write
void NULL_DPI_LR(void *v){}
void NULL_DPI_LW(unsigned char v){}
void NULL_DPI_VR(void *v){}
void NULL_DPI_VW(void *v){}
%}
extern void NULL_DPI_LR(void *v);
extern void NULL_DPI_LW(unsigned char v);
extern void NULL_DPI_VR(void *v);
extern void NULL_DPI_VW(void *v);

{{__SWIG_CONSTANT__}}
