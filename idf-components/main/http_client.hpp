#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "esp_http_client.h"

class HttpClient {
public:
    struct Response {
        std::vector<uint8_t> data;
        int status_code = 0;

        bool ok() const { return status_code >= 200 && status_code < 300; }
        std::string as_string() const { return std::string(data.begin(), data.end()); }
    };

    explicit HttpClient(const std::string& base_url);
    ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    // Synchronous GET. Reuses TCP session across calls. max_size limits body to avoid OOM.
    Response get(const std::string& path, size_t max_size = 256 * 1024);

    // Cleanup the underlying HTTP client. Forces re-init on next get().
    void close();

private:
    void ensure_client(const std::string& url);

    std::string base_url_;
    esp_http_client_handle_t client_ = nullptr;
};
