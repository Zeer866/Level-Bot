#pragma once
#include <functional>

namespace lb {
    void setUploadCallback(std::function<void(bool)> cb);
    void clearUploadCallback();
    void notifyUpload(bool ok);
}
