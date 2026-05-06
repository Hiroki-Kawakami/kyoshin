#include "wifi_setup_screen.hpp"
#include "kyoshin_screen.hpp"
#include "network_manager.hpp"
#include "resources/resources.h"

static const char *rssi_label(int8_t rssi) {
    if (rssi >= -60) return "強";
    if (rssi >= -70) return "良";
    if (rssi >= -80) return "中";
    return "弱";
}

void WiFiSetupScreen::build() {
    // ヘッダー (40px)
    header_ = lv_obj_create(root_);
    lv_obj_remove_style_all(header_);
    lv_obj_align(header_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(header_, 320, 40);
    lv_obj_set_style_border_side(header_, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(header_, 1, 0);
    lv_obj_set_style_border_color(header_, lv_color_hex(0xd3d3d3), 0);

    // コンテンツエリア (200px) — 状態によって中身を入れ替える
    content_ = lv_obj_create(root_);
    lv_obj_remove_style_all(content_);
    lv_obj_align(content_, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_size(content_, 320, 200);
    lv_obj_remove_flag(content_, LV_OBJ_FLAG_SCROLLABLE);
}

void WiFiSetupScreen::onAppear() {
    startScan();
}

// ── スキャン ───────────────────────────────────────────────────────────────

void WiFiSetupScreen::startScan() {
    showScanning();
    network_manager.scanAPs([this](std::vector<NetworkManager::WiFiAP> aps) {
        lv_lock();
        lv_async_call([this, aps = std::move(aps)]() mutable {
            showAPList(aps);
        });
        lv_unlock();
    });
}

void WiFiSetupScreen::showScanning() {
    lv_obj_clean(header_);
    auto title = lv_label_create(header_);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 16, 0);
    lv_label_set_text(title, "WiFi設定");
    lv_obj_set_style_text_font(title, R.font.ipa_24, 0);

    lv_obj_clean(content_);
    lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content_, 12, 0);

    auto spinner = lv_spinner_create(content_);
    lv_obj_set_size(spinner, 48, 48);
    lv_obj_set_style_arc_width(spinner, 5, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 5, LV_PART_INDICATOR);

    auto label = lv_label_create(content_);
    lv_label_set_text(label, "スキャン中...");
    lv_obj_set_style_text_font(label, R.font.ipa_16, 0);
}

// ── APリスト ──────────────────────────────────────────────────────────────

void WiFiSetupScreen::showAPList(const std::vector<NetworkManager::WiFiAP> &aps) {
    lv_obj_clean(content_);
    lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(content_, 0, 0);

    if (aps.empty()) {
        lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(content_, 12, 0);

        auto label = lv_label_create(content_);
        lv_label_set_text(label, "APが見つかりませんでした");
        lv_obj_set_style_text_font(label, R.font.ipa_16, 0);

        auto btn = lv_button_create(content_);
        lv_obj_set_style_pad_hor(btn, 20, 0);
        lv_obj_set_style_pad_ver(btn, 8, 0);
        auto btn_label = lv_label_create(btn);
        lv_label_set_text(btn_label, "再スキャン");
        lv_obj_set_style_text_font(btn_label, R.font.ipa_16, 0);
        lv_obj_add_event_fn(btn, LV_EVENT_CLICKED, [this](lv_event_t *) { startScan(); });
        return;
    }

    lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // スクロール可能なリストコンテナ
    auto list = lv_obj_create(content_);
    lv_obj_remove_style_all(list);
    lv_obj_set_size(list, 320, 200);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_pad_row(list, 0, 0);
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);

    // 再スキャン行
    {
        auto row = lv_obj_create(list);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, LV_PCT(100), 36);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(row, 16, 0);
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(0xd3d3d3), 0);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(row, lv_color_black(), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(row, LV_OPA_10, LV_STATE_PRESSED);

        auto label = lv_label_create(row);
        lv_label_set_text(label, LV_SYMBOL_REFRESH " 再スキャン");
        lv_obj_set_style_text_font(label, R.font.ipa_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x0066cc), 0);
        lv_obj_add_event_fn(row, LV_EVENT_CLICKED, [this](lv_event_t *) { startScan(); });
    }

    // AP行
    for (const auto &ap : aps) {
        auto row = lv_obj_create(list);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, LV_PCT(100), 40);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(row, 16, 0);
        lv_obj_set_style_pad_column(row, 6, 0);
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(0xeeeeee), 0);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(row, lv_color_black(), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(row, LV_OPA_10, LV_STATE_PRESSED);

        auto ssid_label = lv_label_create(row);
        lv_label_set_text(ssid_label, ap.ssid.c_str());
        lv_obj_set_style_text_font(ssid_label, R.font.ipa_16, 0);
        lv_obj_set_flex_grow(ssid_label, 1);

        if (ap.secured) {
            auto lock = lv_image_create(row);
            lv_image_set_src(lock, R.icon.lock_keyhole_16px);
        }

        auto rssi = lv_label_create(row);
        lv_label_set_text(rssi, rssi_label(ap.rssi));
        lv_obj_set_style_text_font(rssi, R.font.ipa_16, 0);
        lv_obj_set_style_text_color(rssi, lv_color_hex(0x888888), 0);

        bool secured = ap.secured;
        std::string ssid = ap.ssid;
        lv_obj_add_event_fn(row, LV_EVENT_CLICKED, [this, ssid, secured](lv_event_t *) {
            if (secured) {
                showPasswordDialog(ssid);
            } else {
                connectToAP(ssid, "");
            }
        });
    }
}

// ── パスワードダイアログ ──────────────────────────────────────────────────

void WiFiSetupScreen::showPasswordDialog(std::string ssid) {
    lv_obj_clean(content_);
    lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(content_, 0, 0);
    lv_obj_set_style_pad_row(content_, 0, 0);

    // タイトル (24px)
    auto title_cont = lv_obj_create(content_);
    lv_obj_remove_style_all(title_cont);
    lv_obj_set_size(title_cont, 320, 24);
    lv_obj_set_flex_flow(title_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(title_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(title_cont, 12, 0);
    lv_obj_set_style_bg_color(title_cont, lv_color_hex(0xf5f5f5), 0);
    lv_obj_set_style_bg_opa(title_cont, LV_OPA_COVER, 0);

    auto title = lv_label_create(title_cont);
    auto title_text = ssid + " のパスワード";
    lv_label_set_text(title, title_text.c_str());
    lv_obj_set_style_text_font(title, R.font.ipa_16, 0);
    lv_obj_set_width(title, 296);
    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);

    // テキストエリア (36px)
    auto ta = lv_textarea_create(content_);
    lv_obj_set_size(ta, 320, 36);
    lv_obj_set_style_text_font(ta, R.font.ipa_16, 0);
    lv_obj_set_style_radius(ta, 0, 0);
    lv_obj_set_style_border_side(ta, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(ta, 1, 0);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_placeholder_text(ta, "パスワードを入力");

    // キーボード (140px) — OK/キャンセルボタン付き
    auto kb = lv_keyboard_create(content_);
    lv_obj_set_size(kb, 320, 140);
    lv_keyboard_set_textarea(kb, ta);

    auto C = [](uint16_t v) constexpr {
        return static_cast<lv_buttonmatrix_ctrl_t>(v);
    };
    auto B = [](uint16_t v) constexpr {
        return static_cast<lv_buttonmatrix_ctrl_t>((LV_BUTTONMATRIX_CTRL_NO_REPEAT | LV_BUTTONMATRIX_CTRL_CLICK_TRIG | LV_BUTTONMATRIX_CTRL_CHECKED) | v);
    };
    auto H = [](uint16_t v) constexpr {
        return static_cast<lv_buttonmatrix_ctrl_t>(LV_BUTTONMATRIX_CTRL_HIDDEN | v);
    };
    static const char * kb_map_lc[] = {
        "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
        " ", "a", "s", "d", "f", "g", "h", "j", "k", "l", " ", "\n",
        "ABC", "z", "x", "c", "v", "b", "n", "m", " ", " ", LV_SYMBOL_BACKSPACE, "\n",
        "1#", LV_SYMBOL_LEFT, " ", " ", ".", ",", "-", "_", " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, nullptr
    };
    static const char * kb_map_uc[] = {
        "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "\n",
        " ", "A", "S", "D", "F", "G", "H", "J", "K", "L", " ", "\n",
        "abc", "Z", "X", "C", "V", "B", "N", "M", " ", " ", LV_SYMBOL_BACKSPACE, "\n",
        "1#", LV_SYMBOL_LEFT, " ", " ", ".", ",", "-", "_", " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, nullptr
    };
    static const lv_buttonmatrix_ctrl_t kb_ctrl_text[] = {
        C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(1),
        H(1), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(2), H(1),
        B(13), C(10), C(10), C(10), C(10), C(10), C(10), C(10), H(1), H(1), B(15),
        B(13), B(8), H(1), C(14), C(10), C(10), C(10), C(10), H(1), B(8), B(15),
    };
    static const char * kb_map_num[] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
        "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "\n",
        " ", "-", "_", "=", "+", "[", "]", ";", ":", LV_SYMBOL_BACKSPACE, "\n",
        "abc", " ", "'", "\"", ",", ".", "/", "?", "\\", " ", LV_SYMBOL_OK, nullptr
    };
    static const lv_buttonmatrix_ctrl_t kb_ctrl_num[] = {
        C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(1),
        C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(1),
        H(2), C(4), C(4), C(4), C(4), C(4), C(4), C(4), C(4), B(6),
        B(13), H(1), C(10), C(10), C(10), C(10), C(10), C(10), C(10), H(1), B(15),
    };
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_LOWER, kb_map_lc, kb_ctrl_text);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_UPPER, kb_map_uc, kb_ctrl_text);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_SPECIAL, kb_map_num, kb_ctrl_num);

    lv_obj_add_event_fn(kb, LV_EVENT_READY, [this, ta, ssid](lv_event_t *) {
        std::string password = lv_textarea_get_text(ta);
        connectToAP(ssid, password);
    });
    lv_obj_add_event_fn(kb, LV_EVENT_CANCEL, [this](lv_event_t *) {
        startScan();
    });

    lv_obj_clean(header_);
    auto back_button = lv_button_create(header_);
    lv_obj_remove_style_all(back_button);
    lv_obj_set_height(back_button, 39);
    lv_obj_align(back_button, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_pad_hor(back_button, 12, 0);
    lv_obj_set_style_bg_color(back_button, lv_color_black(), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(back_button, LV_OPA_20, LV_STATE_PRESSED);
    lv_obj_set_flex_flow(back_button, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(back_button, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(back_button, 12, 0);
    lv_obj_add_event_fn(back_button, LV_EVENT_CLICKED, [this](lv_event_t*){
        startScan();
    });
    auto back_icon = lv_label_create(back_button);
    lv_label_set_text(back_icon, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(back_icon, R.font.ipa_24, 0);
    auto back_label = lv_label_create(back_button);
    lv_label_set_text(back_label, "パスワード入力");
    lv_obj_set_style_text_font(back_label, R.font.ipa_24, 0);

    auto connect_button = lv_button_create(header_);
    lv_obj_align(connect_button, LV_ALIGN_RIGHT_MID, -4, 0);
    lv_obj_add_event_fn(connect_button, LV_EVENT_CLICKED, [this, ta, ssid](lv_event_t *) {
        std::string password = lv_textarea_get_text(ta);
        connectToAP(ssid, password);
    });
    auto connect_label = lv_label_create(connect_button);
    lv_label_set_text(connect_label, "接続");
    lv_obj_set_style_text_font(connect_label, R.font.ipa_16, 0);
}

// ── 接続 ──────────────────────────────────────────────────────────────────

void WiFiSetupScreen::connectToAP(std::string ssid, std::string password) {
    showConnecting(ssid);
    network_manager.connect(ssid, password, [this, ssid](NetworkManager::Result result) {
        lv_lock();
        lv_async_call([this, result, ssid]() {
            handleConnectResult(result, ssid);
        });
        lv_unlock();
    });
}

void WiFiSetupScreen::showConnecting(const std::string &ssid) {
    lv_obj_clean(header_);
    auto title = lv_label_create(header_);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 16, 0);
    lv_label_set_text(title, "WiFi接続");
    lv_obj_set_style_text_font(title, R.font.ipa_24, 0);

    lv_obj_clean(content_);
    lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content_, 12, 0);

    auto spinner = lv_spinner_create(content_);
    lv_obj_set_size(spinner, 48, 48);
    lv_obj_set_style_arc_width(spinner, 5, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 5, LV_PART_INDICATOR);

    auto ssid_label = lv_label_create(content_);
    lv_label_set_text(ssid_label, ("SSID: " + ssid).c_str());
    lv_obj_set_style_text_font(ssid_label, R.font.ipa_16, 0);

    auto status = lv_label_create(content_);
    lv_label_set_text(status, "接続中...");
    lv_obj_set_style_text_font(status, R.font.ipa_16, 0);
    lv_obj_set_style_text_color(status, lv_color_hex(0x888888), 0);
}

void WiFiSetupScreen::handleConnectResult(NetworkManager::Result result, const std::string &ssid) {
    if (result == NetworkManager::Result::Ok) {
        screen_manager.load(std::make_unique<KyoshinScreen>());
        return;
    }
    showError(ssid, result);
}

// ── エラー ────────────────────────────────────────────────────────────────

void WiFiSetupScreen::showError(const std::string &ssid, NetworkManager::Result result) {
    lv_obj_clean(content_);
    lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content_, 12, 0);

    auto title = lv_label_create(content_);
    lv_label_set_text(title, "接続失敗");
    lv_obj_set_style_text_font(title, R.font.ipa_24, 0);

    const char *reason;
    switch (result) {
    case NetworkManager::Result::ApNotFound:  reason = "APが見つかりませんでした"; break;
    case NetworkManager::Result::AuthFailed:  reason = "パスワードが正しくありません"; break;
    case NetworkManager::Result::AssocFailed: reason = "接続に失敗しました"; break;
    case NetworkManager::Result::IpFailed:    reason = "IPアドレスを取得できませんでした"; break;
    default:                                  reason = "エラーが発生しました"; break;
    }

    auto msg = lv_label_create(content_);
    lv_label_set_text(msg, reason);
    lv_obj_set_style_text_font(msg, R.font.ipa_16, 0);
    lv_obj_set_style_text_color(msg, lv_color_hex(0x888888), 0);

    auto btn = lv_button_create(content_);
    lv_obj_set_style_pad_hor(btn, 20, 0);
    lv_obj_set_style_pad_ver(btn, 8, 0);
    auto btn_label = lv_label_create(btn);
    lv_obj_set_style_text_font(btn_label, R.font.ipa_16, 0);

    if (result == NetworkManager::Result::AuthFailed) {
        lv_label_set_text(btn_label, "パスワードを再入力");
        lv_obj_add_event_fn(btn, LV_EVENT_CLICKED, [this, ssid](lv_event_t *) {
            showPasswordDialog(ssid);
        });
    } else {
        lv_label_set_text(btn_label, "再スキャン");
        lv_obj_add_event_fn(btn, LV_EVENT_CLICKED, [this](lv_event_t *) {
            startScan();
        });
    }
}
