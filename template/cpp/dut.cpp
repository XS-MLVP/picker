#include "UT_{{__TOP_MODULE_NAME__}}.hpp"
#include "UT_{{__TOP_MODULE_NAME__}}_dpi.hpp"

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



UT{{__TOP_MODULE_NAME__}}::~UT{{__TOP_MODULE_NAME__}}()
{
    // Finished {{__TOP_MODULE_NAME__}}
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

    xfunction<int, bool> stepfunc = [&](bool d) -> int {
        return this->Step(d);
    };
    this->xclk.ReInit(stepfunc, {}, {&this->port});
}

void UT{{__TOP_MODULE_NAME__}}::initClock(XData &clk)
{
    // initialize {{__TOP_MODULE_NAME__}} clock
    xfunction<int, bool> stepfunc = [&](bool d) -> int {
        return this->Step(d);
    };
    this->xclk.ReInit(stepfunc, {&clk}, {});
}