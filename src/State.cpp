#include "State.hpp"

static std::function<void(bool)> g_callback;

void lb::setUploadCallback(std::function<void(bool)> cb) {
    g_callback = std::move(cb);
}

void lb::clearUploadCallback() {
    g_callback = nullptr;
}

void lb::notifyUpload(bool ok) {
    if (g_callback) g_callback(ok);
}
