#include "dut.hpp"


#if defined(USE_VERILATOR)
DutUnifiedBase::DutUnifiedBase() : DutVerilatorBase() {};
DutUnifiedBase::DutUnifiedBase(int argc, char **argv) : DutVerilatorBase(argc, argv){};
DutUnifiedBase::DutUnifiedBase(char *filename) : DutVerilatorBase(filename) {};
DutUnifiedBase::DutUnifiedBase(char *filename, int argc, char **argv) : DutVerilatorBase(filename, argc, argv) {};
DutUnifiedBase::DutUnifiedBase(std::initializer_list<const char *> args) : DutVerilatorBase(args) {};

#elif defined(USE_VCS)
DutUnifiedBase::DutUnifiedBase() : DutVcsBase() {};
DutUnifiedBase::DutUnifiedBase(int argc, char **argv) : DutVcsBase(argc, argv){};
DutUnifiedBase::DutUnifiedBase(char *filename) : DutVcsBase(filename) {};
DutUnifiedBase::DutUnifiedBase(char *filename, int argc, char **argv) : DutVcsBase(filename, argc, argv) {};
DutUnifiedBase::DutUnifiedBase(std::initializer_list<const char *> args) : DutVcsBase(args) {};
#endif



UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}() : DutUnifiedBase() {
    this->init();
};

UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(int argc, char **argv) : DutUnifiedBase(argc, argv) {
    this->init();
};

UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(char *filename) : DutUnifiedBase(filename) {
    this->init();
};

UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(char *filename, int argc, char **argv) : DutUnifiedBase(filename, argc, argv) {
    this->init();
};

UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(std::initializer_list<const char *> args) : DutUnifiedBase(args){
    this->init();
}


void UT{{__TOP_MODULE_NAME__}}::init()
{
    /*
{{__COMMENTS__}}
    */

    // initialize {{__TOP_MODULE_NAME__}}
{{__XDATA_REINIT__}}

    // bind {{__TOP_MODULE_NAME__}} pins
{{__XDATA_BIND__}}

    // add {{__TOP_MODULE_NAME__}} ports
{{__XPORT_ADD__}}
}

DutUnifiedBase::~DutUnifiedBase()
{
    // finalize {{__TOP_MODULE_NAME__}}
}

UT{{__TOP_MODULE_NAME__}}::~UT{{__TOP_MODULE_NAME__}}()
{
    // finalize {{__TOP_MODULE_NAME__}}
}