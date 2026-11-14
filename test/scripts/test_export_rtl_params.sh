#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd cmake
require_cmd verilator

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
WORK="${ROOT_DIR}/picker_out/RtlParams"

rm -rf "${WORK}"
mkdir -p "${WORK}"

# A required parameter (A, C), a defaulted parameter (B) and a macro-driven
# width: parameters must be pinned in the wrapper, the macro must reach both
# the frontend and the simulator.
cat > "${WORK}/rtl_params.sv" <<'RTL'
module rtl_params #(
    parameter A,
    parameter B = 4,
    parameter C
) (
    input  logic clk,
    input  logic [A-1:0] din,
    input  logic [`RTL_DEF_WIDTH-1:0] wide,
    output logic [B-1:0] dout,
    output logic [`RTL_DEF_WIDTH-1:0] wide_out
);
    assign dout = din[B-1:0];
    assign wide_out = wide;
endmodule
RTL

blue "[rtl-params] Exporting with --rtl-param and --rtl-define"
"${PICKER_BIN}" export "${WORK}/rtl_params.sv" \
    --sname rtl_params \
    --tdir "${WORK}/dut" \
    --autobuild false \
    --rtl-param A=8 \
    --rtl-param C=2 \
    --rtl-define RTL_DEF_WIDTH=16

WRAPPER="${WORK}/dut/rtl_params_top.sv"
test -f "${WRAPPER}"

blue "[rtl-params] Checking the generated wrapper"
# Parameter values travel from the command line to the instantiation.
grep -q "\.A(8)" "${WRAPPER}"
grep -q "\.B(4)" "${WRAPPER}"
grep -q "\.C(2)" "${WRAPPER}"
grep -q "logic \[7:0\] din;" "${WRAPPER}"
grep -q "logic \[3:0\] dout;" "${WRAPPER}"
# The macro width reached the frontend.
grep -q "logic \[15:0\] wide;" "${WRAPPER}"

blue "[rtl-params] Building the generated project"
make -C "${WORK}/dut" -j"$(nproc)"

blue "[rtl-params] Checking filelist macro precedence"
# A filelist that defines the same macro with another value must not produce a
# second definition: the simulator treats differing redefinitions as an error.
printf '+define+RTL_DEF_WIDTH=8\n' > "${WORK}/defs.f"
"${PICKER_BIN}" export "${WORK}/rtl_params.sv" \
    --sname rtl_params \
    --tdir "${WORK}/dut_filelist" \
    --autobuild false \
    --fs "${WORK}/defs.f" \
    --rtl-param A=8 \
    --rtl-param C=2 \
    --rtl-define RTL_DEF_WIDTH=16

test -f "${WORK}/dut_filelist/rtl_params_top.sv"
grep -q "logic \[15:0\] wide;" "${WORK}/dut_filelist/rtl_params_top.sv"
if grep -q "RTL_DEF_WIDTH" "${WORK}/dut_filelist/filelist.f" 2>/dev/null; then
    red "the generated filelist still defines RTL_DEF_WIDTH; REDEFMACRO would fire"
    exit 1
fi
make -C "${WORK}/dut_filelist" -j"$(nproc)"

blue "[rtl-params] Checking that all-default parameters keep the wrapper unchanged"
"${PICKER_BIN}" export "${ROOT_DIR}/example/Adder/Adder.v" \
    --sname Adder \
    --tdir "${WORK}/defaults" \
    --autobuild false
if grep -q " #(" "${WORK}/defaults/Adder_top.sv"; then
    red "an all-default module must not grow a parameter assignment"
    exit 1
fi

blue "[rtl-params] Checking that structured options win over raw passthroughs"
"${PICKER_BIN}" export "${WORK}/rtl_params.sv" \
    --sname rtl_params \
    --tdir "${WORK}/dut_precedence" \
    --autobuild false \
    --rtl-param A=8 \
    --rtl-param C=2 \
    --rtl-define RTL_DEF_WIDTH=16 \
    -S '-DRTL_DEF_WIDTH=4' \
    -S '-GA=3'
grep -q "logic \[7:0\] din;" "${WORK}/dut_precedence/rtl_params_top.sv"
grep -q "logic \[15:0\] wide;" "${WORK}/dut_precedence/rtl_params_top.sv"
grep -q "\.A(8)" "${WORK}/dut_precedence/rtl_params_top.sv"

blue "[rtl-params] Checking rejected inputs"
expect_failure() {
    local message="$1"
    shift
    if "${PICKER_BIN}" export "${WORK}/rtl_params.sv" \
        --sname rtl_params \
        --tdir "${WORK}/rejected" \
        --autobuild false \
        --rtl-define RTL_DEF_WIDTH=16 \
        "$@" > "${WORK}/rejected.log" 2>&1; then
        red "expected a failure: ${message}"
        cat "${WORK}/rejected.log"
        exit 1
    fi
    if ! grep -q "${message}" "${WORK}/rejected.log"; then
        red "missing diagnostic for ${message}"
        cat "${WORK}/rejected.log"
        exit 1
    fi
    rm -rf "${WORK}/rejected"
}

expect_failure "no parameter with this name" --rtl-param A=8 --rtl-param C=2 --rtl-param NOPE=1
expect_failure "expects NAME=VALUE" --rtl-param A=8 --rtl-param C=2 --rtl-param B
expect_failure "hierarchical names are not supported" --rtl-param A=8 --rtl-param C=2 --rtl-param rtl_params.B=8
expect_failure "expects NAME or NAME=VALUE" --rtl-param A=8 --rtl-param C=2 --rtl-define 'BAD-NAME=1'

green "[rtl-params] OK"
