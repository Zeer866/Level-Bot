#include "State.hpp"

static std::function<void(bool)> g_upload;
static std::function<void(const std::string&)> g_account;

void lb::setUploadCallback(std::function<void(bool)> cb) { g_upload = std::move(cb); }
void lb::clearUploadCallback() { g_upload = nullptr; }
void lb::notifyUpload(bool ok) { if (g_upload) g_upload(ok); }

void lb::setAccountCallback(std::function<void(const std::string&)> cb) { g_account = std::move(cb); }
void lb::clearAccountCallback() { g_account = nullptr; }
void lb::notifyAccount(const std::string& response) { if (g_account) g_account(response); }
