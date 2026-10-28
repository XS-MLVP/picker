void VArrTop___024root__trace_chg_0_sub_0(Root* vlSelf, Buffer* bufp) {
    uint32_t* oldp = bufp->oldp(vlSymsp->__Vm_baseCode + 1);
    bufp->chgSData(oldp+0,(vlSelf->ArrTop__DOT__mem[0]),16);
    bufp->chgSData(oldp+1,(vlSelf->ArrTop__DOT__mem[1]),16);
    bufp->chgSData(oldp+2,(vlSelf->ArrTop__DOT__mem[2]),16);
    bufp->chgSData(oldp+3,(vlSelf->ArrTop__DOT__mem[3]),16);
    bufp->chgCData(oldp+4,(vlSelf->ArrTop__DOT__tiny[0]),8);
    bufp->chgCData(oldp+5,(vlSelf->ArrTop__DOT__tiny[1]),8);
    bufp->chgBit(oldp+6,(vlSelf->clk));
    bufp->chgCData(oldp+7,(vlSelf->din),8);
    bufp->chgCData(oldp+8,(vlSelf->dout),8);
}
