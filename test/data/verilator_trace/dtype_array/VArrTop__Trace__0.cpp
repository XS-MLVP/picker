// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Tracing implementation internals

#include "verilated_fst_c.h"
#include "VArrTop__Syms.h"


void VArrTop___024root__trace_chg_0_sub_0(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp);

void VArrTop___024root__trace_chg_0(void* voidSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_chg_0\n"); );
    // Body
    VArrTop___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<VArrTop___024root*>(voidSelf);
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    if (VL_UNLIKELY(!vlSymsp->__Vm_activity)) return;
    VArrTop___024root__trace_chg_0_sub_0((&vlSymsp->TOP), bufp);
}

void VArrTop___024root__trace_chg_dtype____0(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp, uint32_t offset, const VlUnpacked<SData/*15:0*/, 4>& __VdtypeVar);
void VArrTop___024root__trace_chg_dtype____1(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp, uint32_t offset, const VlUnpacked<CData/*7:0*/, 2>& __VdtypeVar);

void VArrTop___024root__trace_chg_0_sub_0(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_chg_0_sub_0\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode + 0);
    bufp->chgBit(oldp+0,(vlSelfRef.clk));
    bufp->chgCData(oldp+1,(vlSelfRef.din),8);
    bufp->chgCData(oldp+2,(vlSelfRef.dout),8);
    VArrTop___024root__trace_chg_dtype____0(vlSelf, bufp, 3, vlSelfRef.ArrTop__DOT__mem);
    VArrTop___024root__trace_chg_dtype____1(vlSelf, bufp, 7, vlSelfRef.ArrTop__DOT__tiny);
}

void VArrTop___024root__trace_chg_dtype____0(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp, uint32_t offset, const VlUnpacked<SData/*15:0*/, 4>& __VdtypeVar) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_chg_dtype____0\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode +  offset);
    bufp->chgSData(oldp+0,(__VdtypeVar[0]),16);
    bufp->chgSData(oldp+1,(__VdtypeVar[1]),16);
    bufp->chgSData(oldp+2,(__VdtypeVar[2]),16);
    bufp->chgSData(oldp+3,(__VdtypeVar[3]),16);
}

void VArrTop___024root__trace_chg_dtype____1(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp, uint32_t offset, const VlUnpacked<CData/*7:0*/, 2>& __VdtypeVar) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_chg_dtype____1\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode +  offset);
    bufp->chgCData(oldp+0,(__VdtypeVar[0]),8);
    bufp->chgCData(oldp+1,(__VdtypeVar[1]),8);
}

void VArrTop___024root__trace_cleanup(void* voidSelf, VerilatedFst* /*unused*/) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_cleanup\n"); );
    // Locals
    VlUnpacked<CData/*0:0*/, 1> __Vm_traceActivity;
    for (int __Vi0 = 0; __Vi0 < 1; ++__Vi0) {
        __Vm_traceActivity[__Vi0] = 0;
    }
    // Body
    VArrTop___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<VArrTop___024root*>(voidSelf);
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    vlSymsp->__Vm_activity = false;
    __Vm_traceActivity[0U] = 0U;
}
