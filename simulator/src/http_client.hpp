#pragma once
#include <string>
#include <vector>
#include <cstdint>

class HttpClient {
public:
    struct Response {
        std::vector<uint8_t> data;
        int status_code = 0;

        bool ok() const { return status_code >= 200 && status_code < 300; }
        std::string as_string() const { return std::string(data.begin(), data.end()); }
    };

    explicit HttpClient(const std::string& base_url);

    // Synchronous GET. max_size limits the response body to avoid OOM (default 256KB).
    Response get(const std::string& path, size_t max_size = 256 * 1024) const;

    void close() {}

private:
    std::string base_url_;
};
