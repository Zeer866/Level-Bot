#pragma once
#include <functional>
#include <string>

namespace lb {
    void setUploadCallback(std::function<void(bool)> cb);
    void clearUploadCallback();
    void notifyUpload(bool ok);

    void setAccountCallback(std::function<void(const std::string&)> cb);
    void clearAccountCallback();
    void notifyAccount(const std::string& response);
}
