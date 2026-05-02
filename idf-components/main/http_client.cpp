#include "http_client.hpp"
#include "esp_log.h"
#include <algorithm>

static const char* TAG = "HttpClient";

struct DownloadCtx {
    std::vector<uint8_t>* buf;
    size_t max_size;
};

static esp_err_t on_event(esp_http_client_event_t* evt) {
    auto* ctx = static_cast<DownloadCtx*>(evt->user_data);
    if (!ctx) return ESP_OK;
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA: {
            if (ctx->buf->empty()) {
                int content_length = esp_http_client_get_content_length(evt->client);
                if (content_length > 0 && (size_t)content_length <= ctx->max_size) {
                    ctx->buf->reserve(content_length);
                }
            }
            size_t remaining = ctx->max_size - ctx->buf->size();
            size_t to_copy = std::min((size_t)evt->data_len, remaining);
            auto* bytes = static_cast<uint8_t*>(evt->data);
            ctx->buf->insert(ctx->buf->end(), bytes, bytes + to_copy);
            break;
        }
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP error");
            break;
        default:
            break;
    }
    return ESP_OK;
}

HttpClient::HttpClient(const std::string& base_url) : base_url_(base_url) {
    if (!base_url_.empty() && base_url_.back() == '/') {
        base_url_.pop_back();
    }
}

HttpClient::~HttpClient() {
    close();
}

void HttpClient::close() {
    if (client_) {
        esp_http_client_cleanup(client_);
        client_ = nullptr;
    }
}

void HttpClient::ensure_client(const std::string& url) {
    if (client_) return;

    esp_http_client_config_t cfg = {};
    cfg.url = url.c_str();
    cfg.event_handler = on_event;
    cfg.buffer_size = 4096;
    cfg.timeout_ms = 15000;
    cfg.skip_cert_common_name_check = true;
    cfg.keep_alive_enable = true;

    client_ = esp_http_client_init(&cfg);
    if (!client_) {
        ESP_LOGE(TAG, "Failed to init HTTP client");
    }
}

HttpClient::Response HttpClient::get(const std::string& path, size_t max_size) {
    std::string url = base_url_;
    if (!path.empty() && path.front() != '/') {
        url += '/';
    }
    url += path;

    ensure_client(url);

    Response resp;
    if (!client_) return resp;

    DownloadCtx ctx{&resp.data, max_size};
    esp_http_client_set_url(client_, url.c_str());
    esp_http_client_set_user_data(client_, &ctx);

    esp_err_t err = esp_http_client_perform(client_);
    if (err == ESP_OK) {
        resp.status_code = esp_http_client_get_status_code(client_);
        ESP_LOGI(TAG, "GET %s -> %d (%zu bytes)", url.c_str(), resp.status_code, resp.data.size());
    } else {
        ESP_LOGE(TAG, "GET %s failed: %s", url.c_str(), esp_err_to_name(err));
        // Re-init on next call since the connection state is unknown
        close();
    }

    return resp;
}
