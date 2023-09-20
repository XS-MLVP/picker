#include <bits/stdc++.h>
#include "mvcontext.h"

extern "C"
{
    class MVTop
    {
    public:
        void *topp_;
        MVContext *contextp_;
        explicit MVTop();
        explicit MVTop(MVContext *contextp);
        // explicit MVTop(MVContext *contextp, const char *vcname);
        virtual ~MVTop();
        void eval() { eval_step(); }
        /// Evaluate when calling multiple units/models per time step.
        void eval_step();
        /// Evaluate at end of a timestep for tracing, when using eval_step().
        /// Application must call after all eval() and before time changes.
        void eval_end_step();
        /// Simulation complete, run final blocks.  Application must call on completion.
        void final();
        /// Are there scheduled events to handle?
        bool eventsPending();
        /// Returns time at next time slot. Aborts if !eventsPending()
        uint64_t nextTimeSlot();
        /// Retrieve name of this model instance (as passed to constructor).
        const char *name() const;

        // Abstract methods from VerilatedModel
        const char *hierName() const;
        const char *modelName() const;
        unsigned threads() const;
    };
}