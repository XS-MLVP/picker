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
        void commandArgs(int argc, char **argv);
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
    };
}