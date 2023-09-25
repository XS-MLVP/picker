#pragma once

#include <bits/stdc++.h>

extern "C"
{

    class MVWaveFile
    {
    public:
        void *wavefilep_;
        MVWaveFile() ;
        /// Close and destruct
        ~MVWaveFile() ;
        /// Open a file with given filename
        bool open(const char* name);
        /// Close object's file
        void close();
        /// Write data to file (if it is open)
        ssize_t write(const char *bufp, ssize_t len);
    };

    class MVWaveC
    {

    public:
        void *tracep_; // TraceObject

        /// Construct the dump. Optional argument is a preconstructed file.
        explicit MVWaveC(MVWaveFile *filep = new MVWaveFile());
        /// Destruct, flush, and close the dump
        ~MVWaveC();

        // METHODS - User called

        /// Return if file is open
        bool isOpen() const;
        /// Open a new VCD file
        /// This includes a complete header dump each time it is called,
        /// just as if this object was deleted and reconstructed.
        void open(const char *filename);
        /// Continue a VCD dump by rotating to a new file name
        /// The header is only in the first file created, this allows
        /// "cat" to be used to combine the header plus any number of data files.
        void openNext(bool incFilename = true);
        /// Set size in bytes after which new file should be created
        /// This will create a header file, followed by each separate file
        /// which might be larger than the given size (due to chunking and
        /// alignment to a start of a given time's dump).  Any file but the
        /// first may be removed.  Cat files together to create viewable vcd.
        void rolloverSize(size_t size);
        /// Close dump
        void close();
        /// Flush dump
        void flush();
        /// Write one cycle of dump data
        /// Call with the current context's time just after eval'ed,
        /// e.g. ->dump(contextp->time())
        void dump(uint64_t timeui);
        /// Write one cycle of dump data - backward compatible and to reduce
        /// conversion warnings.  It's better to use a uint64_t time instead.
        void dump(double timestamp);
        void dump(uint32_t timestamp);
        void dump(int timestamp);

        // METHODS - Internal/backward compatible
        // \protectedsection

        // Set time units (s/ms, defaults to ns)
        // Users should not need to call this, as for Verilated models, these
        // propage from the Verilated default timeunit
        void set_time_unit(const char *unit);
        // Set time resolution (s/ms, defaults to ns)
        // Users should not need to call this, as for Verilated models, these
        // propage from the Verilated default timeprecision
        void set_time_resolution(const char *unit);
        // Set variables to dump, using $dumpvars format
        // If level = 0, dump everything and hier is then ignored
        void dumpvars(int level, const char *hier);
    };
}