#pragma once
#include <cmath>
#include <string>
#include <optional>
#include "screen_manager.hpp"
#include "kyoshin_settings.hpp"
#include "resources/resources.h"

class SettingsPage: public Screen {
protected:
    lv_obj_t *list_;

    void createBackButton(const char *title);
    void createList();
    void addListRow(
        const char *title,
        std::optional<std::function<void(lv_obj_t *row, lv_obj_t *col)>> factory,
        std::optional<std::function<void()>> on_click = std::nullopt);
    void addSwitchRow(
        const char *title,
        bool value,
        std::function<void(bool)> on_change);
    void addDropdownRow(
        const char *title,
        const char *options,
        int selected,
        std::optional<int> width,
        std::function<void(int)> on_change);
    void addSliderRow(
        const char *title,
        std::optional<std::function<std::string(int value)>> subtitle,
        int min_value,
        int max_value,
        int initial_value,
        std::function<void(int, bool)> on_change);

    void addSeparator();
};
