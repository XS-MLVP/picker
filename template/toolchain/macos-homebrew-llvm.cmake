if (NOT APPLE OR HOMEBREW_LLVM_TOOLCHAIN_INITIALIZED)
    return()
endif ()

set(HOMEBREW_LLVM_TOOLCHAIN_INITIALIZED TRUE CACHE INTERNAL "")

find_package(PkgConfig REQUIRED)
# zlib
pkg_check_modules(ZLIB REQUIRED zlib)
include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS})
# systemc
pkg_check_modules(SYSTEMC REQUIRED systemc)
include_directories(${SYSTEMC_INCLUDE_DIRS})
link_directories(${SYSTEMC_LIBRARY_DIRS})
# macOS need this, according to https://github.com/verilator/verilator/blob/master/include/verilated.mk.in#L114-L118
add_link_options("-Wl,-U,__Z15vl_time_stamp64v,-U,__Z13sc_time_stampv")

# Detect homebrew llvm
execute_process(
        COMMAND "brew" "--prefix" "llvm"
        OUTPUT_VARIABLE LLVM_PREFIX
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (EXISTS "${LLVM_PREFIX}")
    message(STATUS "Using Homebrew LLVM: ${LLVM_PREFIX}")

    set(CMAKE_C_COMPILER   "${LLVM_PREFIX}/bin/clang"   CACHE FILEPATH "" FORCE)
    set(CMAKE_CXX_COMPILER "${LLVM_PREFIX}/bin/clang++" CACHE FILEPATH "" FORCE)

    # Set lld
    execute_process(
            COMMAND "brew" "--prefix" "--installed" "lld"
            OUTPUT_VARIABLE LLD_PREFIX
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT EXISTS "${LLD_PREFIX}")
        set(LLD_PREFIX "${LLVM_PREFIX}")
    endif ()
    if (EXISTS "${LLD_PREFIX}/bin/ld64.lld")
        message(STATUS "Using lld linker: ${LLD_PREFIX}/bin/ld64.lld")
        add_link_options(-fuse-ld=lld)
    endif ()

    include_directories("${LLVM_PREFIX}/include/c++/v1")
    link_directories("${LLVM_PREFIX}/lib/c++" "${LLVM_PREFIX}/lib")

else ()
    message(WARNING "Homebrew LLVM not found — using system Clang.")
endif ()
