if (NOT APPLE OR HOMEBREW_LLVM_TOOLCHAIN_INITIALIZED)
    return()
endif ()

set(HOMEBREW_LLVM_TOOLCHAIN_INITIALIZED TRUE CACHE INTERNAL "")

# Detect homebrew llvm
execute_process(
        COMMAND "brew" "--prefix" "--installed" "llvm"
        OUTPUT_VARIABLE LLVM_PREFIX
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (EXISTS "${LLVM_PREFIX}")
    message(STATUS "Using Homebrew LLVM: ${LLVM_PREFIX}")

    set(CMAKE_C_COMPILER "${LLVM_PREFIX}/bin/clang" CACHE FILEPATH "" FORCE)
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
    set(MACOS_HOMEBREW_LLVM_LINKER_FLAGS
            "-L${LLVM_PREFIX}/lib/c++"
            "-L${LLVM_PREFIX}/lib"
            "-Wl,-rpath,${LLVM_PREFIX}/lib/c++"
            "-Wl,-rpath,${LLVM_PREFIX}/lib"
            "-lc++ -lc++abi"
    )
    list(JOIN MACOS_HOMEBREW_LLVM_LINKER_FLAGS " " LINKER_FLAGS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}" CACHE STRING "" FORCE)
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LINKER_FLAGS}" CACHE STRING "" FORCE)

else ()
    message(STATUS "Homebrew LLVM not found — using system Clang.")
endif ()
