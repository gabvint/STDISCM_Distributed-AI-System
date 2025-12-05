#include "tesseract_wrapper.h"

#include <stdexcept>
#include <mutex>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

namespace {
    tesseract::TessBaseAPI g_tess;
    bool g_initialized = false;
    std::mutex g_tessMutex;   // NEW
}

bool init_tesseract(const std::string& data_path, const std::string& lang) {
    if (g_initialized) return true;

    if (g_tess.Init(data_path.empty() ? nullptr : data_path.c_str(),
                    lang.c_str()) != 0) {
        return false;
    }
    g_initialized = true;
    return true;
}

std::string run_tesseract_on_png_bytes(const std::string& png_data) {
    if (!g_initialized) {
        throw std::runtime_error("Tesseract not initialized. Call init_tesseract first.");
    }
    std::lock_guard<std::mutex> lock(g_tessMutex);

    const l_uint8* data_ptr = reinterpret_cast<const l_uint8*>(png_data.data());
    size_t data_size = png_data.size();

    PIX* pix = pixReadMemPng(data_ptr, data_size);
    if (!pix) {
        throw std::runtime_error("Failed to create PIX from PNG data.");
    }

    g_tess.SetImage(pix);
    char* out_text = g_tess.GetUTF8Text();
    pixDestroy(&pix);

    if (!out_text) {
        throw std::runtime_error("Tesseract returned null text.");
    }

    std::string result(out_text);
    delete[] out_text;
    return result;
}
