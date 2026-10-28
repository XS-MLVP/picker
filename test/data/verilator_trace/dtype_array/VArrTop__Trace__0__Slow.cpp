// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Tracing implementation internals

#include "verilated_fst_c.h"
#include "VArrTop__Syms.h"


VL_ATTR_COLD void VArrTop___024root__trace_init_dtype____0(VArrTop___024root* vlSelf, VerilatedFst* tracep, const char* name, uint32_t fidx, uint32_t c, VerilatedTraceSigDirection direction);
VL_ATTR_COLD void VArrTop___024root__trace_init_dtype____1(VArrTop___024root* vlSelf, VerilatedFst* tracep, const char* name, uint32_t fidx, uint32_t c, VerilatedTraceSigDirection direction);

VL_ATTR_COLD void VArrTop___024root__trace_init_sub__TOP__0(VArrTop___024root* vlSelf, VerilatedFst* tracep) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_init_sub__TOP__0\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    const int c = vlSymsp->__Vm_baseCode;
    VL_TRACE_PUSH_PREFIX(tracep, "$rootio", VerilatedTracePrefixType::SCOPE_MODULE, 0, 0);
    VL_TRACE_DECL_BIT(tracep,c+0,0,"clk",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC);
    VL_TRACE_DECL_BUS(tracep,c+1,0,"din",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, 7,0);
    VL_TRACE_DECL_BUS(tracep,c+2,0,"dout",-1, VerilatedTraceSigDirection::OUTPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, 7,0);
    VL_TRACE_POP_PREFIX(tracep);
    VL_TRACE_PUSH_PREFIX(tracep, "ArrTop", VerilatedTracePrefixType::SCOPE_MODULE, 0, 0);
    VL_TRACE_DECL_BIT(tracep,c+0,0,"clk",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC);
    VL_TRACE_DECL_BUS(tracep,c+1,0,"din",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, 7,0);
    VL_TRACE_DECL_BUS(tracep,c+2,0,"dout",-1, VerilatedTraceSigDirection::OUTPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, 7,0);

    VArrTop___024root__trace_init_dtype____0(vlSelf, tracep, "mem", 0, c+3, VerilatedTraceSigDirection::NONE);

    VArrTop___024root__trace_init_dtype____1(vlSelf, tracep, "tiny", 0, c+7, VerilatedTraceSigDirection::NONE);
    VL_TRACE_POP_PREFIX(tracep);
}

VL_ATTR_COLD void VArrTop___024root__trace_init_dtype_sub____0(VArrTop___024root* vlSelf, VerilatedFst* tracep, const char* name, uint32_t fidx, uint32_t c, VerilatedTraceSigDirection direction);

VL_ATTR_COLD void VArrTop___024root__trace_init_dtype____0(VArrTop___024root* vlSelf, VerilatedFst* tracep, const char* name, uint32_t fidx, uint32_t c, VerilatedTraceSigDirection direction) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_init_dtype____0\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    VArrTop___024root__trace_init_dtype_sub____0(vlSelf, tracep, name, fidx, c, direction);
}

VL_ATTR_COLD void VArrTop___024root__trace_init_dtype_sub____0(VArrTop___024root* vlSelf, VerilatedFst* tracep, const char* name, uint32_t fidx, uint32_t c, VerilatedTraceSigDirection direction) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_init_dtype_sub____0\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    VL_TRACE_PUSH_PREFIX(tracep, name, VerilatedTracePrefixType::ARRAY_UNPACKED, 0, 3);
    for (int i = 0; i < 4; ++i) {
        VL_TRACE_DECL_BUS_ARRAY(tracep,c+0+i*1,fidx,"",-1, direction, VerilatedTraceSigKind::VAR, VerilatedTraceSigType::LOGIC, (i + 0), 15,0);
    }
    VL_TRACE_POP_PREFIX(tracep);
}

VL_ATTR_COLD void VArrTop___024root__trace_init_dtype_sub____1(VArrTop___024root* vlSelf, VerilatedFst* tracep, const char* name, uint32_t fidx, uint32_t c, VerilatedTraceSigDirection direction);

VL_ATTR_COLD void VArrTop___024root__trace_init_dtype____1(VArrTop___024root* vlSelf, VerilatedFst* tracep, const char* name, uint32_t fidx, uint32_t c, VerilatedTraceSigDirection direction) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_init_dtype____1\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    VArrTop___024root__trace_init_dtype_sub____1(vlSelf, tracep, name, fidx, c, direction);
}

VL_ATTR_COLD void VArrTop___024root__trace_init_dtype_sub____1(VArrTop___024root* vlSelf, VerilatedFst* tracep, const char* name, uint32_t fidx, uint32_t c, VerilatedTraceSigDirection direction) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_init_dtype_sub____1\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    VL_TRACE_PUSH_PREFIX(tracep, name, VerilatedTracePrefixType::ARRAY_UNPACKED, 0, 1);
    for (int i = 0; i < 2; ++i) {
        VL_TRACE_DECL_BUS_ARRAY(tracep,c+0+i*1,fidx,"",-1, direction, VerilatedTraceSigKind::VAR, VerilatedTraceSigType::LOGIC, (i + 0), 7,0);
    }
    VL_TRACE_POP_PREFIX(tracep);
}

VL_ATTR_COLD void VArrTop___024root__trace_init_top(VArrTop___024root* vlSelf, VerilatedFst* tracep) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_init_top\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    VArrTop___024root__trace_init_sub__TOP__0(vlSelf, tracep);
}

VL_ATTR_COLD void VArrTop___024root__trace_const_0(void* voidSelf, VerilatedFst::Buffer* bufp);
VL_ATTR_COLD void VArrTop___024root__trace_full_0(void* voidSelf, VerilatedFst::Buffer* bufp);
void VArrTop___024root__trace_chg_0(void* voidSelf, VerilatedFst::Buffer* bufp);
void VArrTop___024root__trace_cleanup(void* voidSelf, VerilatedFst* /*unused*/);

VL_ATTR_COLD void VArrTop___024root__trace_register(VArrTop___024root* vlSelf, VerilatedFst* tracep) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_register\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    tracep->addConstCb(&VArrTop___024root__trace_const_0, 0, vlSelf);
    tracep->addFullCb(&VArrTop___024root__trace_full_0, 0, vlSelf);
    tracep->addChgCb(&VArrTop___024root__trace_chg_0, 0, vlSelf);
    tracep->addCleanupCb(&VArrTop___024root__trace_cleanup, vlSelf);
}

VL_ATTR_COLD void VArrTop___024root__trace_const_0(void* voidSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_const_0\n"); );
    // Body
    VArrTop___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<VArrTop___024root*>(voidSelf);
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
}

VL_ATTR_COLD void VArrTop___024root__trace_full_0_sub_0(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp);

VL_ATTR_COLD void VArrTop___024root__trace_full_0(void* voidSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_full_0\n"); );
    // Body
    VArrTop___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<VArrTop___024root*>(voidSelf);
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VArrTop___024root__trace_full_0_sub_0((&vlSymsp->TOP), bufp);
}

VL_ATTR_COLD void VArrTop___024root__trace_full_dtype____0(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp, uint32_t offset, const VlUnpacked<SData/*15:0*/, 4>& __VdtypeVar);
VL_ATTR_COLD void VArrTop___024root__trace_full_dtype____1(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp, uint32_t offset, const VlUnpacked<CData/*7:0*/, 2>& __VdtypeVar);

VL_ATTR_COLD void VArrTop___024root__trace_full_0_sub_0(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_full_0_sub_0\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode);
    bufp->fullBit(oldp+0,(vlSelfRef.clk));
    bufp->fullCData(oldp+1,(vlSelfRef.din),8);
    bufp->fullCData(oldp+2,(vlSelfRef.dout),8);
    VArrTop___024root__trace_full_dtype____0(vlSelf, bufp, 3, vlSelfRef.ArrTop__DOT__mem);
    VArrTop___024root__trace_full_dtype____1(vlSelf, bufp, 7, vlSelfRef.ArrTop__DOT__tiny);
}

VL_ATTR_COLD void VArrTop___024root__trace_full_dtype____0(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp, uint32_t offset, const VlUnpacked<SData/*15:0*/, 4>& __VdtypeVar) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_full_dtype____0\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode + offset);
    bufp->fullSData(oldp+0,(__VdtypeVar[0]),16);
    bufp->fullSData(oldp+1,(__VdtypeVar[1]),16);
    bufp->fullSData(oldp+2,(__VdtypeVar[2]),16);
    bufp->fullSData(oldp+3,(__VdtypeVar[3]),16);
}

VL_ATTR_COLD void VArrTop___024root__trace_full_dtype____1(VArrTop___024root* vlSelf, VerilatedFst::Buffer* bufp, uint32_t offset, const VlUnpacked<CData/*7:0*/, 2>& __VdtypeVar) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VArrTop___024root__trace_full_dtype____1\n"); );
    VArrTop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode + offset);
    bufp->fullCData(oldp+0,(__VdtypeVar[0]),8);
    bufp->fullCData(oldp+1,(__VdtypeVar[1]),8);
}
