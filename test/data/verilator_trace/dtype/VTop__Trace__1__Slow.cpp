void VTop___024root__trace_full_dtype____2(VTop___024root* vlSelf, Buffer* bufp, uint32_t offset, const Array& __VdtypeVar) {
    VL_DEBUG_IF(VL_DBG_MSGF("brace } and escaped quote \" in debug string\n"); );
    // A closing brace } in a comment is not the end of this helper.
    uint32_t* oldp = bufp->oldp(vlSymsp->__Vm_baseCode + offset);
    bufp->fullSData(oldp+0, (__VdtypeVar[0]), 16);
    bufp->fullSData(oldp+1, (__VdtypeVar[1]), 16);
    bufp->fullSData(oldp+9, (__VdtypeVar[9]), 16);
}
void VTop___024root__trace_init_dtype_sub____2(VTop___024root* vlSelf, Trace* tracep, const char* name, uint32_t fidx, uint32_t c, Direction direction) {
    VL_TRACE_PUSH_PREFIX(tracep, name, ARRAY_UNPACKED, 0, 15);
    VL_TRACE_DECL_BIT(tracep,c+9,0,"uninstantiated",-1, NONE, VAR, LOGIC);
    VL_TRACE_POP_PREFIX(tracep);
}
