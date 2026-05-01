#include "http_client.hpp"
#include <curl/curl.h>
#include <algorithm>
#include <cstdio>

struct WriteCtx {
    std::vector<uint8_t>* buf;
    size_t max_size;
};

static size_t on_write(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* ctx = static_cast<WriteCtx*>(userdata);
    size_t total = size * nmemb;
    size_t remaining = ctx->max_size - ctx->buf->size();
    size_t to_copy = std::min(total, remaining);
    ctx->buf->insert(ctx->buf->end(), ptr, ptr + to_copy);
    return total;
}

HttpClient::HttpClient(const std::string& base_url) : base_url_(base_url) {
    if (!base_url_.empty() && base_url_.back() == '/') {
        base_url_.pop_back();
    }
}

HttpClient::Response HttpClient::get(const std::string& path, size_t max_size) const {
    std::string url = base_url_;
    if (!path.empty() && path.front() != '/') {
        url += '/';
    }
    url += path;

    Response resp;
    WriteCtx ctx{&resp.data, max_size};

    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "[HttpClient] curl_easy_init failed\n");
        return resp;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        long status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        resp.status_code = static_cast<int>(status);
        printf("[HttpClient] GET %s -> %d (%zu bytes)\n", url.c_str(), resp.status_code, resp.data.size());
    } else {
        fprintf(stderr, "[HttpClient] GET %s failed: %s\n", url.c_str(), curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    return resp;
}
