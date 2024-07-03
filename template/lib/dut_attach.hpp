#pragma once
#include <bits/stdc++.h>
#include <svdpi.h>
#include <dlfcn.h>

#include "verilated.h"
#include "V{{__TOP_MODULE_NAME__}}.h"

extern "C" {
    V{{__TOP_MODULE_NAME__}}* dlcreates(VerilatedContext* vc);
}