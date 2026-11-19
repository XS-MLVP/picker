// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Tracing implementation internals

#include "verilated_fst_c.h"
#include "VAdder__Syms.h"


void VAdder___024root__trace_chg_0_sub_0(VAdder___024root* vlSelf, VerilatedFst::Buffer* bufp);

void VAdder___024root__trace_chg_0(void* voidSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VAdder___024root__trace_chg_0\n"); );
    // Body
    VAdder___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<VAdder___024root*>(voidSelf);
    VAdder__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    if (VL_UNLIKELY(!vlSymsp->__Vm_activity)) return;
    VAdder___024root__trace_chg_0_sub_0((&vlSymsp->vlInst.TOP), bufp);
}

void VAdder___024root__trace_chg_0_sub_0(VAdder___024root* vlSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VAdder___024root__trace_chg_0_sub_0\n"); );
    VAdder__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Locals
    ::VlWide<4>/*127:0*/ __Vtemp_1;
    ::VlWide<4>/*127:0*/ __Vtemp_2;
    ::VlWide<4>/*127:0*/ __Vtemp_3;
    ::VlWide<5>/*159:0*/ __Vtemp_5;
    ::VlWide<5>/*159:0*/ __Vtemp_6;
    ::VlWide<5>/*159:0*/ __Vtemp_7;
    ::VlWide<5>/*159:0*/ __Vtemp_8;
    ::VlWide<5>/*159:0*/ __Vtemp_9;
    // Body
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode + 0);
    bufp->chgWData(oldp+0,(vlSelfRef.Adder_top__DOT__a),128);
    bufp->chgWData(oldp+4,(vlSelfRef.Adder_top__DOT__b),128);
    bufp->chgBit(oldp+8,(vlSelfRef.Adder_top__DOT__cin));
    bufp->chgWData(oldp+9,(vlSelfRef.Adder_top__DOT__sum),128);
    bufp->chgBit(oldp+13,(vlSelfRef.Adder_top__DOT__cout));
    __Vtemp_1[0U] = vlSelfRef.Adder_top__DOT__cin;
    __Vtemp_1[1U] = 0U;
    __Vtemp_1[2U] = 0U;
    __Vtemp_1[3U] = 0U;
    VL_ADD_W(4, __Vtemp_2, vlSelfRef.Adder_top__DOT__b, __Vtemp_1);
    VL_ADD_W(4, __Vtemp_3, vlSelfRef.Adder_top__DOT__a, __Vtemp_2);
    bufp->chgWData(oldp+14,(__Vtemp_3),128);
    __Vtemp_5[0U] = vlSelfRef.Adder_top__DOT__a[0U];
    __Vtemp_5[1U] = vlSelfRef.Adder_top__DOT__a[1U];
    __Vtemp_5[2U] = vlSelfRef.Adder_top__DOT__a[2U];
    __Vtemp_5[3U] = vlSelfRef.Adder_top__DOT__a[3U];
    __Vtemp_5[4U] = 0U;
    __Vtemp_6[0U] = vlSelfRef.Adder_top__DOT__cin;
    __Vtemp_6[1U] = 0U;
    __Vtemp_6[2U] = 0U;
    __Vtemp_6[3U] = 0U;
    __Vtemp_6[4U] = 0U;
    __Vtemp_7[0U] = vlSelfRef.Adder_top__DOT__b[0U];
    __Vtemp_7[1U] = vlSelfRef.Adder_top__DOT__b[1U];
    __Vtemp_7[2U] = vlSelfRef.Adder_top__DOT__b[2U];
    __Vtemp_7[3U] = vlSelfRef.Adder_top__DOT__b[3U];
    __Vtemp_7[4U] = 0U;
    VL_ADD_W(5, __Vtemp_8, __Vtemp_6, __Vtemp_7);
    VL_ADD_W(5, __Vtemp_9, __Vtemp_5, __Vtemp_8);
    bufp->chgBit(oldp+18,((1U & __Vtemp_9[4U])));
}

void VAdder___024root__trace_cleanup(void* voidSelf, VerilatedFst* /*unused*/) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VAdder___024root__trace_cleanup\n"); );
    // Locals
    ::VlUnpacked<::CData/*0:0*/, 1> __VtraceActivity;
    for (int __Vi0 = 0; __Vi0 < 1; ++__Vi0) {
        __VtraceActivity[__Vi0] = 0;
    }
    // Body
    VAdder___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<VAdder___024root*>(voidSelf);
    VAdder__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    vlSymsp->__Vm_activity = false;
    __VtraceActivity[0U] = 0U;
}
