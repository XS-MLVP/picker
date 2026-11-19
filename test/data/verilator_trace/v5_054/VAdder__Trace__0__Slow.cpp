// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Tracing implementation internals

#include "verilated_fst_c.h"
#include "VAdder__Syms.h"


VL_ATTR_COLD void VAdder___024root__trace_init_sub__TOP__0(VAdder___024root* vlSelf, VerilatedFst* tracep) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VAdder___024root__trace_init_sub__TOP__0\n"); );
    VAdder__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    const int c = vlSymsp->__Vm_baseCode;
    VL_TRACE_PUSH_PREFIX(tracep, "Adder_top", VerilatedTracePrefixType::SCOPE_MODULE, 0, 0);
    VL_TRACE_DECL_WIDE(tracep,c+0,0,"a",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::VAR, VerilatedTraceSigType::LOGIC, 127,0);
    VL_TRACE_DECL_WIDE(tracep,c+4,0,"b",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::VAR, VerilatedTraceSigType::LOGIC, 127,0);
    VL_TRACE_DECL_BIT(tracep,c+8,0,"cin",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::VAR, VerilatedTraceSigType::LOGIC);
    VL_TRACE_DECL_WIDE(tracep,c+9,0,"sum",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::VAR, VerilatedTraceSigType::LOGIC, 127,0);
    VL_TRACE_DECL_BIT(tracep,c+13,0,"cout",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::VAR, VerilatedTraceSigType::LOGIC);
    VL_TRACE_PUSH_PREFIX(tracep, "Adder", VerilatedTracePrefixType::SCOPE_MODULE, 0, 0);
    VL_TRACE_DECL_BUS(tracep,c+19,0,"WIDTH",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::PARAMETER, VerilatedTraceSigType::BIT, 31,0);
    VL_TRACE_DECL_WIDE(tracep,c+0,0,"a",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, 127,0);
    VL_TRACE_DECL_WIDE(tracep,c+4,0,"b",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, 127,0);
    VL_TRACE_DECL_BIT(tracep,c+8,0,"cin",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC);
    VL_TRACE_DECL_WIDE(tracep,c+14,0,"sum",-1, VerilatedTraceSigDirection::OUTPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, 127,0);
    VL_TRACE_DECL_BIT(tracep,c+18,0,"cout",-1, VerilatedTraceSigDirection::OUTPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC);
    VL_TRACE_POP_PREFIX(tracep);
    VL_TRACE_POP_PREFIX(tracep);
}

VL_ATTR_COLD void VAdder___024root__trace_init_top(VAdder___024root* vlSelf, VerilatedFst* tracep) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VAdder___024root__trace_init_top\n"); );
    VAdder__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    VAdder___024root__trace_init_sub__TOP__0(vlSelf, tracep);
}

VL_ATTR_COLD void VAdder___024root__trace_const_0(void* voidSelf, VerilatedFst::Buffer* bufp);
VL_ATTR_COLD void VAdder___024root__trace_full_0(void* voidSelf, VerilatedFst::Buffer* bufp);
void VAdder___024root__trace_chg_0(void* voidSelf, VerilatedFst::Buffer* bufp);
void VAdder___024root__trace_cleanup(void* voidSelf, VerilatedFst* /*unused*/);

VL_ATTR_COLD void VAdder___024root__trace_register(VAdder___024root* vlSelf, VerilatedFst* tracep) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VAdder___024root__trace_register\n"); );
    VAdder__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    tracep->addConstCb(&VAdder___024root__trace_const_0, 0, vlSelf);
    tracep->addFullCb(&VAdder___024root__trace_full_0, 0, vlSelf);
    tracep->addChgCb(&VAdder___024root__trace_chg_0, 0, vlSelf);
    tracep->addCleanupCb(&VAdder___024root__trace_cleanup, vlSelf);
}

VL_ATTR_COLD void VAdder___024root__trace_const_0_sub_0(VAdder___024root* vlSelf, VerilatedFst::Buffer* bufp);

VL_ATTR_COLD void VAdder___024root__trace_const_0(void* voidSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VAdder___024root__trace_const_0\n"); );
    // Body
    VAdder___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<VAdder___024root*>(voidSelf);
    VAdder__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VAdder___024root__trace_const_0_sub_0((&vlSymsp->vlInst.TOP), bufp);
}

VL_ATTR_COLD void VAdder___024root__trace_const_0_sub_0(VAdder___024root* vlSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VAdder___024root__trace_const_0_sub_0\n"); );
    VAdder__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode);
    bufp->fullIData(oldp+19,(0x00000080U),32);
}

VL_ATTR_COLD void VAdder___024root__trace_full_0_sub_0(VAdder___024root* vlSelf, VerilatedFst::Buffer* bufp);

VL_ATTR_COLD void VAdder___024root__trace_full_0(void* voidSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VAdder___024root__trace_full_0\n"); );
    // Body
    VAdder___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<VAdder___024root*>(voidSelf);
    VAdder__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VAdder___024root__trace_full_0_sub_0((&vlSymsp->vlInst.TOP), bufp);
}

VL_ATTR_COLD void VAdder___024root__trace_full_0_sub_0(VAdder___024root* vlSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VAdder___024root__trace_full_0_sub_0\n"); );
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
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode);
    bufp->fullWData(oldp+0,(vlSelfRef.Adder_top__DOT__a),128);
    bufp->fullWData(oldp+4,(vlSelfRef.Adder_top__DOT__b),128);
    bufp->fullBit(oldp+8,(vlSelfRef.Adder_top__DOT__cin));
    bufp->fullWData(oldp+9,(vlSelfRef.Adder_top__DOT__sum),128);
    bufp->fullBit(oldp+13,(vlSelfRef.Adder_top__DOT__cout));
    __Vtemp_1[0U] = vlSelfRef.Adder_top__DOT__cin;
    __Vtemp_1[1U] = 0U;
    __Vtemp_1[2U] = 0U;
    __Vtemp_1[3U] = 0U;
    VL_ADD_W(4, __Vtemp_2, vlSelfRef.Adder_top__DOT__b, __Vtemp_1);
    VL_ADD_W(4, __Vtemp_3, vlSelfRef.Adder_top__DOT__a, __Vtemp_2);
    bufp->fullWData(oldp+14,(__Vtemp_3),128);
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
    bufp->fullBit(oldp+18,((1U & __Vtemp_9[4U])));
}
