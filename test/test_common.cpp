#include "picker.hpp"
#include <string>
#include <cstdlib>
#include <map>

// Globals from picker.cpp needed by tests
picker::main_opts main_opts;
picker::export_opts export_opts;
picker::pack_opts pack_opts;
char *picker::lib_random_hash = nullptr;

// These are required by libparser.a, which is linked by many tests.
// We provide dummy/default values here for the test environment.
std::map<std::string, std::string> lang_lib_map  = {{"cpp", "lib"},
                                                    {"java", "java/xspcomm-java.jar"},
                                                    {"scala", "scala/xspcomm-scala.jar"},
                                                    {"python", "python"},
                                                    {"golang", "golang"},
                                                    {"lua", "lua/luaxspcomm.so"}};
std::map<std::string, std::string> display_names = {{"cpp", "Cpp"},       {"java", "Java"},     {"scala", "Scala"},
                                                    {"python", "Python"}, {"golang", "Golang"}, {"lua", "Lua"}};

namespace picker {
    // Default to false for tests, can be overridden by specific test files if needed.
    bool is_debug = false;
}

// Stubs for functions from picker.cpp that might be called by libraries
void check_debug()
{
    if (std::getenv("PICKER_DEBUG") != NULL) { picker::is_debug = true; }
}

std::string to_base62(uint64_t num)
{
    const std::string base62_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string result;
    do {
        result = base62_chars[num % 62] + result;
        num >>= 6;
    } while (num > 0);
    return result;
}

// Mock implementations for appimage functions from src/appimage/configuration.cpp
// These prevent linker errors in tests that link against libraries using these functions.
namespace picker::appimage {
    bool is_running_as_appimage() {
        // Return false for tests as they are not running as an AppImage
        return false;
    }
    std::string get_temporary_path() {
        // Provide a standard temporary path for the test environment
        return "/tmp";
    }
    void initialize() {
        // Do nothing for tests
    }
    std::string get_template_path() {
        return "";
    }
    std::string get_picker_lib(const std::string &lib_lang) {
        return "";
    }
}
