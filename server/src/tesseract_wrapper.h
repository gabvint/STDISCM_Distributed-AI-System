#pragma once
#include <string>

bool init_tesseract(const std::string& data_path, const std::string& lang);
std::string run_tesseract_on_png_bytes(const std::string& png_data);
