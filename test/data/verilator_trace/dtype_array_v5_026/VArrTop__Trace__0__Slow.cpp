// Minimized Verilator 5.026 FST output for the same one-dimensional arrays
// as dtype_array/. The old format declares arrays inline rather than in dtype helpers.
void VArrTop___024root__trace_init_sub__TOP__0(Root* vlSelf, Trace* tracep) {
    tracep->declBit(c+7,0,"clk",-1, INPUT, WIRE, LOGIC, false,-1);
    tracep->declBus(c+8,0,"din",-1, INPUT, WIRE, LOGIC, false,-1, 7,0);
    tracep->declBus(c+9,0,"dout",-1, OUTPUT, WIRE, LOGIC, false,-1, 7,0);
    tracep->pushPrefix("ArrTop", VerilatedTracePrefixType::SCOPE_MODULE);
    tracep->declBit(c+7,0,"clk",-1, INPUT, WIRE, LOGIC, false,-1);
    tracep->declBus(c+8,0,"din",-1, INPUT, WIRE, LOGIC, false,-1, 7,0);
    tracep->declBus(c+9,0,"dout",-1, OUTPUT, WIRE, LOGIC, false,-1, 7,0);
    tracep->pushPrefix("mem", VerilatedTracePrefixType::ARRAY_UNPACKED);
    for (int i = 0; i < 4; ++i) {
        tracep->declBus(c+1+i*1,0,"",-1, NONE, VAR, LOGIC, true,(i+0), 15,0);
    }
    tracep->popPrefix();
    tracep->pushPrefix("tiny", VerilatedTracePrefixType::ARRAY_UNPACKED);
    for (int i = 0; i < 2; ++i) {
        tracep->declBus(c+5+i*1,0,"",-1, NONE, VAR, LOGIC, true,(i+0), 7,0);
    }
    tracep->popPrefix();
    tracep->popPrefix();
}
void VArrTop___024root__trace_init_top(Root* vlSelf, Trace* tracep) {
    VArrTop___024root__trace_init_sub__TOP__0(vlSelf, tracep);
}
void VArrTop___024root__trace_full_0_sub_0(Root* vlSelf, Buffer* bufp) {
    uint32_t* oldp = bufp->oldp(vlSymsp->__Vm_baseCode);
    bufp->fullSData(oldp+1,(vlSelf->ArrTop__DOT__mem[0]),16);
    bufp->fullSData(oldp+2,(vlSelf->ArrTop__DOT__mem[1]),16);
    bufp->fullSData(oldp+3,(vlSelf->ArrTop__DOT__mem[2]),16);
    bufp->fullSData(oldp+4,(vlSelf->ArrTop__DOT__mem[3]),16);
    bufp->fullCData(oldp+5,(vlSelf->ArrTop__DOT__tiny[0]),8);
    bufp->fullCData(oldp+6,(vlSelf->ArrTop__DOT__tiny[1]),8);
    bufp->fullBit(oldp+7,(vlSelf->clk));
    bufp->fullCData(oldp+8,(vlSelf->din),8);
    bufp->fullCData(oldp+9,(vlSelf->dout),8);
}
