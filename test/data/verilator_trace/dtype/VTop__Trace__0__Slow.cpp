VL_TRACE_PUSH_PREFIX(tracep, "Top", VerilatedTracePrefixType::SCOPE_MODULE, 0, 0);
VL_TRACE_DECL_BIT(tracep,c+8,0,"train_ready",-1, INPUT, WIRE, LOGIC);
VL_TRACE_DECL_BIT(tracep,c+9,0,"train_valid",-1, OUTPUT, WIRE, LOGIC);
VL_TRACE_DECL_BUS(tracep,c+100,0,"first_zero",-1, NONE, VAR, LOGIC, 15,0);
VL_TRACE_DECL_BUS(tracep,c+109,0,"first_nine",-1, NONE, VAR, LOGIC, 15,0);
VL_TRACE_DECL_BUS(tracep,c+200,0,"second_zero",-1, NONE, VAR, LOGIC, 15,0);
VL_TRACE_POP_PREFIX(tracep);
void VTop___024root__trace_full_dtype____2(VTop___024root* vlSelf, Buffer* bufp, uint32_t offset, const Array& __VdtypeVar);
void VTop___024root__trace_full_sub(VTop___024root* vlSelf, Buffer* bufp) {
    uint32_t* oldp = bufp->oldp(vlSymsp->__Vm_baseCode);
    bufp->fullBit(oldp+8, vlSelfRef.Top__DOT__ready);
    bufp->fullBit(oldp+9, vlSelfRef.Top__DOT__valid);
    VTop___024root__trace_full_dtype____2(vlSelf, bufp, 100U, vlSelfRef.Top__DOT__first);
    VTop___024root__trace_full_dtype____2(vlSelf, bufp, 0xc8U, vlSelfRef.Top__DOT__second);
}
