#include <bits/stdc++.h>
#include <bits/fs_fwd.h>
#include "picker.hpp"

namespace picker::appimage {

/*
    * AppImage support explaination:
    * This file contains functions to initialize the application image configuration, extract the template and library
    * directories from the application image.
    * The application image configuration is used to store the template and library directories, allowing for easy
   access
    * and management of these resources.

    * How it works:
    * 1. The `initialize()` function is called at the start of the application to extract the necessary template and
   library
    *    paths.
    * 2. It checks if the template and library directories exist in the file system.
    * 3. If they do not exist, it extracts them from the application image.
    * 4. The `get_template_path()` function returns the path to the template directory based on the XDG Base Directory
    *    Specification, which is typically located at "$HOME/.config/picker/$version/template".
    * 5. The `get_picker_lib()` function returns the path to the picker library, which is typically located at
    *    "$HOME/.local/share/picker/lib/$version/lib".
    * 6. The `extract_template()` and `extract_library()` functions copy the template and library directories from the
    *    application image to the user's home directory if they do not already exist.
    * 7. The paths are constructed using the environment variable `HOME`
    *
    * Note:
    * - The `GIT_HASH` is a placeholder for the version of the application, which should be replaced with the actual
   version
    *   during the build process.
    * - The rendered project structure will use LD_LIBRARY_PATH to find the libraries in the specified path, which is
   typically
    *   located at "$HOME/.local/share/picker/lib/$version/lib".
    * - The rendered project structure will use the template path to find the templates, which is typically located at
    *   "$HOME/.config/picker/$version/template" and defined in the Makefile.
    * - Picker will check /proc/self/exe symlink to check whether it is running as an AppImage or not.
*/

bool is_running_as_appimage()
{
    // AppImage will extract the binary to temporary directory and symlink it to /proc/self/exe. AppImage mode is on if
    // /proc/self/exe points to a file in /tmp
    std::string exe_path;
    try {
        exe_path = std::filesystem::read_symlink("/proc/self/exe").string();
    } catch (const std::filesystem::filesystem_error &e) {
        PK_FATAL("Failed to read symlink /proc/self/exe: %s", e.what());
        return false;
    }
    return exe_path.find("/tmp") != std::string::npos;
}

// Extract the template directory from the application image
void extract_template()
{
    // get self binary path from /proc/self/exe symlink
    std::string picker_path = std::filesystem::read_symlink("/proc/self/exe").string();
    // get the directory of the picker binary
    std::string picker_dir = std::filesystem::path(picker_path).parent_path().parent_path().string() + "/share/picker/";

    // copy template directory from the picker binary directory to the user's home directory
    std::string user_home     = getenv("HOME") ? getenv("HOME") : ".";
    std::string template_path = user_home + "/.config/picker/" + GIT_HASH + "/template";
    if (!std::filesystem::exists(template_path)) { std::filesystem::create_directories(template_path); }
    // copy the template directory from the picker binary directory to the user's home directory
    std::filesystem::copy(picker_dir + "template", template_path,
                          std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    PK_DEBUG("Template directory extracted to: %s\n", template_path.c_str());
}

// Extract the library directory from the application image
void extract_library()
{
    // get self binary path from /proc/self/exe symlink
    std::string picker_path = std::filesystem::read_symlink("/proc/self/exe").string();
    // get the directory of the picker binary
    std::string picker_dir = std::filesystem::path(picker_path).parent_path().parent_path().string() + "/share/picker/";

    // copy library directory from the picker binary directory to the user's home directory
    std::string user_home = getenv("HOME") ? getenv("HOME") : ".";
    std::string lib_path  = user_home + "/.local/share/picker/lib/" + GIT_HASH + "/lib";
    if (!std::filesystem::exists(lib_path)) { std::filesystem::create_directories(lib_path); }
    // copy the library directory from the picker binary directory to the user's home directory
    std::filesystem::copy(picker_dir + "lib", lib_path,
                          std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    PK_DEBUG("Library directory extracted to: %s\n", lib_path.c_str());
}

void initialize()
{
    // Initialize the application image configuration
    // This function is called at the start of the application to extract the necessary template and library paths

    // Check if the template directory is exist in file system
    std::string template_path = get_template_path();
    if (std::filesystem::exists(template_path)) {
        PK_DEBUG("Template path: %s\n", template_path.c_str());
    } else {
        PK_MESSAGE("Template path %s is not found", template_path.c_str());
        PK_MESSAGE("Extract the template directory from the application image");
        extract_template();
    }

    // Check if the library directory is exist in file system
    std::string lib_path = get_picker_lib("lib");
    if (std::filesystem::exists(lib_path)) {
        PK_DEBUG("Library path: %s\n", lib_path.c_str());
    } else {
        PK_MESSAGE("Library path %s is not found", lib_path.c_str());
        PK_MESSAGE("Extract the library directory from the application image");
        extract_library();
    }

    PK_DEBUG("Application image configuration initialized.\n");
    return;
}

std::string get_template_path()
{
    // Return the path to the template directory
    // It's extracted from the application image follow XDG Base Directory Specification such as
    // "$HOME/.config/picker/$version/template". For now, it returns a hardcoded path, but it can be modified to search
    // in specific directories
    std::string user_home     = getenv("HOME") ? getenv("HOME") : ".";
    std::string template_path = user_home + "/.config/picker/" + GIT_HASH + "/template";
    return template_path;
}

std::string get_picker_lib(const std::string &lib_lang)
{
    // This function should return the path to the picker library, default should be in xspcomm.
    // For now, it returns a hardcoded path, but it can be modified to search in specific directories
    std::string user_home = getenv("HOME") ? getenv("HOME") : ".";
    std::string lib_path  = user_home + "/.local/share/picker/lib/" + GIT_HASH + "/" + lib_lang;
    return lib_path;
}



} // namespace picker::appimage