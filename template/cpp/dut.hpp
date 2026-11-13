#pragma once
#include "dut_base.hpp"
#include "xspcomm/xcomm.h"
#include <stdexcept>
#include <tuple>
using namespace xspcomm;
{% if __HAS_UNPACKED_ARRAYS__ %}
#include <type_traits>

#ifndef PICKER_SIGNAL_ARRAY_VIEW_HPP
#define PICKER_SIGNAL_ARRAY_VIEW_HPP
// Non-owning leaves reference the DUT's bound XData, with checked RTL indexing.
template<typename T>
class PickerSignalArray {
    int left_ = 0, right_ = -1;
    std::vector<T> elements_;
public:
    PickerSignalArray() = default;
    PickerSignalArray(int left, int right, std::initializer_list<T> elements)
        : left_(left), right_(right), elements_(elements) {}

    int left() const { return left_; }
    int right() const { return right_; }
    size_t size() const { return elements_.size(); }

    auto &operator[](int index) {
        const int64_t offset = (int64_t(index) - left_) * (left_ <= right_ ? 1 : -1);
        if (offset < 0) { throw std::out_of_range("RTL array index out of range"); }
        auto &element = elements_.at(size_t(offset));
        if constexpr (std::is_pointer_v<T>) { return *element; }
        else { return element; }
    }
    const auto &operator[](int index) const {
        return const_cast<PickerSignalArray *>(this)->operator[](index);
    }
};
#endif
{% endif %}
// clang-format off
class UT{{__TOP_MODULE_NAME__}}
{
    void init();
public:
    /*
{{__COMMENTS__}}
    */

    // {{__TOP_MODULE_NAME__}} 
{{__XDATA_DECLARATION__}}

    // signal hashmap
    std::map<std::tuple<std::string, int, bool>, XData*> signal_map;
    std::map<std::string, std::vector<std::shared_ptr<XData>>> signal_list_map;
    {% if __HAS_AGGREGATE_VIEWS__ %}
    std::shared_ptr<XVPI> vpi_backend;
    {% endif %}

    // DUT
    DutUnifiedBase *dut = nullptr;

    // Ports for binding
    XPort xport;

    // Sub ports
{{__XPORT_CASCADED_DEC__}}

    // Clock for DUT timing
    XClock xclock;

    XSignalCFG *xcfg = nullptr;

    // {{__TOP_MODULE_NAME__}}
    UT{{__TOP_MODULE_NAME__}}();
    UT{{__TOP_MODULE_NAME__}}(int argc, char **argv);
    UT{{__TOP_MODULE_NAME__}}(char *filename);
    UT{{__TOP_MODULE_NAME__}}(char *filename, int argc, char **argv);
    UT{{__TOP_MODULE_NAME__}}(std::initializer_list<const char *> args);
    ~UT{{__TOP_MODULE_NAME__}}();

    // Funtions for DUT
    void InitClock(XData &clk);
    // clang-format on

    /*******************************/
    /**      User APIs             */
    /*******************************/
    void InitClock(std::string name)
    {
        this->InitClock(this->xport[name]);
    }
    void Step(int i = 1)
    {
        this->xclock.Step(i);
    }
    void StepRis(std::function<void(uint64_t, void *)> fc, void *args = nullptr)
    {
        this->xclock.StepRis(fc, args);
    }
    void StepFal(std::function<void(uint64_t, void *)> fc, void *args = nullptr)
    {
        this->xclock.StepFal(fc, args);
    }
    XClock* GetXClock()
    {
        return &this->xclock;
    }
    XPort* GetXPort()
    {
        return &this->xport;
    }
    bool ResumeWaveformDump()
    {
        return this->dut->ResumeWaveformDump();
    }
    bool PauseWaveformDump()
    {
        return this->dut->PauseWaveformDump();
    }
    int WaveformPaused()
    {
        return this->dut->WaveformPaused();
    }
    void SetWaveform(std::string filename)
    {
        this->dut->SetWaveform((const char *)filename.c_str());
    }
    std::string GetWaveFormat()
    {
        return this->dut->GetWaveFormat();
    } 
    void FlushWaveform()
    {
        this->dut->FlushWaveform();
    }
    void SetCoverage(std::string filename)
    {
        this->dut->SetCoverage((const char *)filename.c_str());
    }
    void ResetCoverage()
    {
        this->dut->ResetCoverage();
    }
    int GetCovMetrics()
    {
        return this->dut->GetCovMetrics();
    }
    void CheckPoint(std::string filename)
    {
        this->dut->CheckPoint((const char *)filename.c_str());
    }
    void Restore(std::string filename)
    {
        this->dut->Restore((const char *)filename.c_str());
    }
    XData *GetInternalSignal(std::string name, int index = -1, bool use_vpi = false)
    {
        const auto key = std::make_tuple(name, index, use_vpi);
        if (this->signal_map.find(key) == this->signal_map.end()) {
            XData *signal = nullptr;
            if(!use_vpi){
                std::string xname = "CFG:" + name;
                if (this->dut->GetXSignalCFGBasePtr()==0)return nullptr;
                if(index != -1){
                    signal = this->xcfg->NewXData(name, index, xname);
                }else{
                    signal = this->xcfg->NewXData(name, xname);
                }
            }else{
                uint64_t vpi_handle = this->dut->GetVPIHandleObj(name);
                signal = XData::FromVPI(vpi_handle, this->dut->GetVPIFuncPtr("vpi_get"),
                                     this->dut->GetVPIFuncPtr("vpi_get_value"),
                                     this->dut->GetVPIFuncPtr("vpi_put_value"), "VPI:"+name);
#if defined(USE_UVS)
                if (!disable_uvs_mem_direct) {
                            if (signal != nullptr && vpi_handle != 0) {
                                typedef uint64_t (*vpi_get_address_t)(uint64_t);
                                auto get_addr = (vpi_get_address_t)this->dut->GetVPIFuncPtr("uvs_get_address");
                                if (get_addr != nullptr) {
                                    uint64_t addr = get_addr(vpi_handle);
                                    if (addr != 0) {
                                        signal->BindMixFromUvs(addr);
                                    }
                                }
                            }
                        }
#endif
            }
            if(signal == nullptr){
                return nullptr;
            }
            this->signal_map[key] = signal;
        }
        return this->signal_map[key];
    }
    {% if __HAS_AGGREGATE_VIEWS__ %}
    XView GetInternalSignalView(std::string name)
    {
        if (!this->vpi_backend) {
            this->vpi_backend = std::make_shared<XVPI>();
            this->vpi_backend->Configure(
                this->dut->GetVPIFuncPtr("vpi_get"),
                this->dut->GetVPIFuncPtr("vpi_get_value"),
                this->dut->GetVPIFuncPtr("vpi_put_value"),
                this->dut->GetVPIFuncPtr("vpi_handle_by_name"),
                this->dut->GetVPIFuncPtr("vpi_handle_by_index"),
                this->dut->GetVPIFuncPtr("vpi_handle"),
                this->dut->GetVPIFuncPtr("vpi_iterate"),
                this->dut->GetVPIFuncPtr("vpi_scan"),
                this->dut->GetVPIFuncPtr("vpi_get_str"),
                this->dut->GetVPIFuncPtr("vpi_release_handle"),
                this->dut->GetVPIScope(), {{__AGGREGATE_LAYOUTS_JSON__}});
        }
        return this->vpi_backend->Get(name);
    }
    {% endif %}
    std::vector<std::shared_ptr<XData>> GetInternalSignal(std::string name, bool is_array){
        Assert(is_array==true, "GetInternalSignal: is_array must be true");
        if(this->signal_list_map.find(name) == this->signal_list_map.end()){
            std::vector<std::shared_ptr<XData>> signal = this->xcfg->NewXDataArray(name, "CFG:" + name);
            if(signal.size() == 0){
                return signal;
            }
            this->signal_list_map[name] = signal;
        }
        return this->signal_list_map[name];
    }
    std::vector<std::string> GetInternalSignalList(std::string prefix = "", int deep = 99, bool use_vpi = false){
        if(!use_vpi){
            return this->xcfg->GetSignalNames(prefix);
        }
        return this->dut->VPIInternalSignalList(prefix, deep);
    }
    std::vector<std::string> VPIInternalSignalList(std::string name = "", int depth = 99)
    {
        return this->dut->VPIInternalSignalList(name, depth);
    }
    void Finish()
    {
        {% if __HAS_AGGREGATE_VIEWS__ %}
        if (this->vpi_backend) {
            this->vpi_backend->Close();
            this->vpi_backend.reset();
        }
        {% endif %}
        this->dut->Finish();
    }
    void RefreshComb()
    {
        this->xclock.RefreshComb();
    }
    void AtClone()
    {
        this->dut->atClone();
    }
    /*******************************/
    /**    End of User APIs        */
    /*******************************/
};
