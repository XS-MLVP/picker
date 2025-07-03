#ifndef PICKER_APPIMAGE_CONFIGURATION_HPP
#define PICKER_APPIMAGE_CONFIGURATION_HPP

#include <string>

namespace picker::appimage {

bool is_running_as_appimage();

void initialize();

std::string get_template_path();

std::string get_picker_lib(const std::string &lib_lang);

} // namespace picker::appimage

#endif // PICKER_APPIMAGE_CONFIGURATION_HPP