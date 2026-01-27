#include "UT_{{__TOP_MODULE_NAME__}}.hpp"
#include "UT_{{__TOP_MODULE_NAME__}}_dpi.hpp"

UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}() {
    this->dut = new DutUnifiedBase();
    this->init();
};

UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(int argc, char **argv) {
    this->dut = new DutUnifiedBase(argc, argv);
    this->init();
};

UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(char *filename) {
    this->dut = new DutUnifiedBase(filename);
    this->init();
};

UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(char *filename, int argc, char **argv){
    this->dut = new DutUnifiedBase(filename, argc, argv);
    this->init();
};

UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(std::initializer_list<const char *> args){
    this->dut = new DutUnifiedBase(args);
    this->init();
}


UT{{__TOP_MODULE_NAME__}}::~UT{{__TOP_MODULE_NAME__}}()
{
    delete this->dut;
    for (auto const& [key, val] : this->signal_map) {
        delete val;
    }
    this->signal_map.clear();
    delete this->xcfg;
    this->xcfg = nullptr;
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
        return this->dut->simStep(d);
    };
    this->xclock.ReInit(stepfunc, {}, {});
    this->xclock.Add(this->xport);
    this->xcfg = new XSignalCFG(this->dut->GetXSignalCFGPath(), this->dut->GetXSignalCFGBasePtr());

    // create subport
{{__XPORT_CASCADED_SGN__}}

}

void UT{{__TOP_MODULE_NAME__}}::InitClock(XData &clk)
{
    this->xclock.Add(clk);
}
