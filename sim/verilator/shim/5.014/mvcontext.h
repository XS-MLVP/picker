#pragma once

#include <bits/stdc++.h>

extern "C"
{
    class MVContext
    {
    public:
        /// @brief VerilatedContext
        void *contextp_;

        explicit MVContext();
        virtual ~MVContext();

        /// Record command-line arguments, for retrieval by $test$plusargs/$value$plusargs,
        /// and for parsing +verilator+ run-time arguments.
        /// This should be called before the first model is created.
        void commandArgs(int argc, const char **argv);
        /// Add a command-line argument to existing arguments
        void commandArgsAdd(int argc, const char **argv);

        void gotError(bool flag);
        /// Return if got a $stop or non-fatal error
        bool gotError();
        /// Set if got a $finish or $stop/error
        void gotFinish(bool flag);
        /// Return if got a $finish or $stop/error
        bool gotFinish() const;
        /// Select initial value of otherwise uninitialized signals.
        /// 0 = Set to zeros
        /// 1 = Set all bits to one
        /// 2 = Randomize all bits
        void randReset(int val);
        /// Return randReset value
        int randReset();
        /// Return default random seed
        void randSeed(int val);
        /// Set default random seed, 0 = seed it automatically
        int randSeed() const;

        uint64_t time() const;
        /// Set current simulation time. See time() for side effect details
        void time(uint64_t value);
        /// Advance current simulation time. See time() for side effect details
        /// NOT Multi-thread safe! Use only when single threaded.
        void timeInc(uint64_t add);
        /// Return time units as power-of-ten
        int timeunit() const;
        /// Set time units as power-of-ten
        void timeunit(int value);
        /// Return time units as IEEE-standard text
        const char *timeunitString() const;
        /// Get time precision as power-of-ten
        int timeprecision() const;
        /// Return time precision as power-of-ten
        void timeprecision(int value);
        /// Get time precision as IEEE-standard text
        const char *timeprecisionString() const;

        /// Get number of threads used for simulation (including the main thread)
        unsigned threads() const;
        /// Set number of threads used for simulation (including the main thread)
        /// Can only be called before the thread pool is created (before first model is added).
        void threads(unsigned n);

        /// Allow traces to at some point be enabled (disables some optimizations)
        void traceEverOn(bool flag);
    };
}