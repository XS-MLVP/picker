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
    VArrTop___024root__trace_chg_0_sub_0((&vlSymsp->vlInst.TOP), bufp);
}

void VArrTop___024root__trace_chg_dtype____0(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp, uint32_t offset, const ::VlUnpacked<::CData/*7:0*/, 4>& __VdtypeVar);

void VArrTop___024root__trace_chg_0_sub_0(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_chg_0_sub_0\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode + 0);
    if (VL_UNLIKELY((vlSelfRef.__VtraceActivity[1U]))) {
        bufp->chgCData(oldp+0,(vlSelfRef.ArrTop_top__DOT__ArrTop__DOT__shadow__BRA__2__KET__),8);
        VArrTop___024root__trace_chg_dtype____0(vlSelf, bufp, 1, vlSelfRef.ArrTop_top__DOT__ArrTop__DOT__mem);
        bufp->chgCData(oldp+5,(vlSelfRef.ArrTop_top__DOT__ArrTop__DOT__shadow[0]),8);
        bufp->chgCData(oldp+6,(vlSelfRef.ArrTop_top__DOT__ArrTop__DOT__shadow[1]),8);
        bufp->chgCData(oldp+7,(vlSelfRef.ArrTop_top__DOT__ArrTop__DOT__shadow[2]),8);
        bufp->chgCData(oldp+8,(vlSelfRef.ArrTop_top__DOT__ArrTop__DOT__shadow[3]),8);
    }
    bufp->chgBit(oldp+9,(vlSelfRef.ArrTop_top__DOT__clk));
    bufp->chgCData(oldp+10,(vlSelfRef.ArrTop_top__DOT__din),8);
    bufp->chgCData(oldp+11,(vlSelfRef.ArrTop_top__DOT__dout),8);
}

void VArrTop___024root__trace_chg_dtype____0(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp, uint32_t offset, const ::VlUnpacked<::CData/*7:0*/, 4>& __VdtypeVar) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_chg_dtype____0\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode +  offset);
    bufp->chgCData(oldp+0,(__VdtypeVar[0]),8);
    bufp->chgCData(oldp+1,(__VdtypeVar[1]),8);
    bufp->chgCData(oldp+2,(__VdtypeVar[2]),8);
    bufp->chgCData(oldp+3,(__VdtypeVar[3]),8);
}

void VArrTop___024root__trace_cleanup(void* voidSelf, VerilatedFst* /*unused*/) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_cleanup\n"); );
    // Body
    VArrTop___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<VArrTop___024root*>(voidSelf);
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    vlSymsp->__Vm_activity = false;
    vlSymsp->vlInst.TOP.__VtraceActivity[0U] = 0U;
    vlSymsp->vlInst.TOP.__VtraceActivity[1U] = 0U;
}
