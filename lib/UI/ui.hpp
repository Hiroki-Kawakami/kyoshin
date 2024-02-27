/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#define UI_IMPL_BEGIN namespace UI {
#define UI_IMPL_END }

#include <optional>
#include <memory>
#include <string>
#include "M5Unified.h"
#include "types.hpp"
#include "rtos.hpp"

UI_IMPL_BEGIN

using std::string;
using std::optional;
using std::shared_ptr;
using std::weak_ptr;

class Scene : public std::enable_shared_from_this<Scene>, private NoMove {
protected:
    optional<weak_ptr<Scene>> parentScene;
    shared_ptr<Scene> childScene;
    bool needsDisplay = true;
    bool loaded = false;
    friend void run();

public:
    // Lifecycle
    virtual void didLoad();
    virtual void willAppear();
    virtual void didAppear();
    virtual void willDisappear();
    virtual void didDisappear();

    // Event Loop
    virtual void eventLoop();

    // Drawing
    void setNeedsDisplay();
    bool displayIfNeeded();
    virtual void display();
    void clearButton(int index = 0, int width = 3, int color = TFT_BLACK);
    void drawButton(int index, int width, string text, int color = TFT_WHITE, int backgroundColor = TFT_BLACK);

    // Transition
    void presentScene(shared_ptr<Scene> scene);
    void dismissScene();
    void swapScene(shared_ptr<Scene> scene);
};

extern shared_ptr<Scene> activeScene;
bool isUITask();
void init();
void run();
void send(std::function<void(void)> func);
void setRootScene(shared_ptr<Scene> scene);

struct ListItem {
    string title;
    string value;
    string button;
};

class ListScene : public UI::Scene {
protected:
    int selectedRow = 0;
    int displayOffset = 0;
    virtual void eventLoop() override;
    virtual void display() override;
    void reloadData();
    void drawItem(int index, M5Canvas &drawBuffer, int width, int height);

public:
    virtual int numberOfRows() = 0;
    virtual void itemForRow(int row, ListItem &item) = 0;
    virtual void itemSelected(int index) {}
};

class ModalScene : public UI::Scene {
protected:
    uint16_t centerX, centerY, modalWidth, modalHeight;
    int backgroundColor = TFT_BLACK;
    virtual void display() override;
};

UI_IMPL_END
