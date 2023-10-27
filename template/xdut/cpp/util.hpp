#pragma once
#include <bits/stdc++.h>
#include <execinfo.h>

namespace dut
{
    void inline Traceback()
    {
        const int size = 200;
        void *buffer[size];
        char **strings;
        int nptrs = backtrace(buffer, size);
        printf("backtrace() returned %d address\n", nptrs);
        strings = backtrace_symbols(buffer, nptrs);
        if (strings)
        {
            for (int i = 0; i < nptrs; ++i)
            {
                printf("%s\n", strings[i]);
            }
            free(strings);
        }
    }

    enum class LogLevel
    {
        debug = 1,
        info,
        warning,
        error,
        fatal,
    };

    LogLevel get_log_level();
    void set_log_level(LogLevel val);

#define output(o, level, prefix, fmt, ...)                         \
    {                                                              \
        if (level >= get_log_level())                              \
        {                                                          \
            fprintf(o, "[%s:%d] %s ", __FILE__, __LINE__, prefix); \
            fprintf(o, fmt, ##__VA_ARGS__);                        \
            fprintf(o, "%s\n", "");                                \
        }                                                          \
    }
#define Info(fmt, ...)                                                 \
    {                                                                  \
        output(stdout, LogLevel::info, "[ info]", fmt, ##__VA_ARGS__); \
    }
#define Warn(fmt, ...)                                                    \
    {                                                                     \
        output(stdout, LogLevel::warning, "[ warn]", fmt, ##__VA_ARGS__); \
    }
#define Error(fmt, ...)                                                 \
    {                                                                   \
        output(stderr, LogLevel::error, "[error]", fmt, ##__VA_ARGS__); \
    }
#define Debug(fmt, ...)                                                 \
    {                                                                   \
        output(stdout, LogLevel::debug, "[debug]", fmt, ##__VA_ARGS__); \
    }
#define Fatal(fmt, ...)                                                 \
    {                                                                   \
        output(stderr, LogLevel::fatal, "[fatal]", fmt, ##__VA_ARGS__); \
        exit(-1);                                                       \
    }

#define Assert(c, fmt, ...)                                                       \
    {                                                                             \
        if (!(c))                                                                 \
        {                                                                         \
            output(stderr, LogLevel::fatal, "[assert fail]", fmt, ##__VA_ARGS__); \
            Traceback();                                                          \
            exit(-1);                                                             \
        }                                                                         \
    }

    template <typename... Args>
    std::string sFmt(const std::string &format, Args... args)
    {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
        if (size_s <= 0)
        {
            throw std::runtime_error("Error during formatting.");
        }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
    }

    inline bool sWith(const std::string &str, const std::string &prefix)
    {
        return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
    }

    template <typename T>
    inline bool contians(std::vector<T> v, T a)
    {
        for (T x : v)
        {
            if (x == a)
                return true;
        }
        return false;
    }

    inline std::string sLower(std::string input)
    {
        auto str = input;
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        return str;
    }

#define likely(cond) __builtin_expect(cond, 1)
#define unlikely(cond) __builtin_expect(cond, 0)

}
