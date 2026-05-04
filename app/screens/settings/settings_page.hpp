#pragma once
#include <optional>
#include "screen_manager.hpp"
#include "resources/resources.h"

class SettingsPage: public Screen {
protected:
    lv_obj_t *list_;

    void createBackButton(const char *title);
    void createList();
    void addListRow(
        const char *title,
        std::function<void(lv_obj_t *row, lv_obj_t *col)> factory,
        std::optional<std::function<void()>> on_click = std::nullopt);
    lv_obj_t *createDropdown(
        lv_obj_t *parent,
        const char *options,
        int selected,
        std::function<void(int)> on_change);
};
