/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#include <algorithm>
#include "ui.hpp"

UI_IMPL_BEGIN

shared_ptr<Scene> rootScene;
shared_ptr<Scene> activeScene;

#define FRAME (33 / portTICK_PERIOD_MS)
class UITask : public RTOS::Task<FRAME> {
    using RTOS::Task<FRAME>::Task;
    virtual void cycle() override;
};
optional<UITask> uiTask = std::nullopt;

bool isUITask() {
    return uiTask.has_value() && uiTask->isCurrentTask();
}

static void drawActiveScene(bool force = false) {
    if (!activeScene) return;
    if (force) activeScene->setNeedsDisplay();
    activeScene->displayIfNeeded();
}

void run() {
    uiTask.emplace();
    uiTask->createQueue();
    if (!rootScene->loaded) {
        rootScene->didLoad();
        rootScene->loaded = true;
    }
    rootScene->willAppear();
    activeScene = rootScene;
    drawActiveScene(true);
    rootScene->didAppear();
    uiTask->run();
}

void send(std::function<void(void)> func) {
    if (uiTask.has_value()) uiTask->send(func);
}

void UITask::cycle() {
    if (activeScene) {
        M5.update();
        activeScene->eventLoop();
        drawActiveScene();
    }
}

void setRootScene(shared_ptr<Scene> scene) {
    rootScene = scene;
}

void Scene::didLoad() {}
void Scene::willAppear() {}
void Scene::didAppear() {}
void Scene::willDisappear() {}
void Scene::didDisappear() {}

void Scene::eventLoop() {
    if (M5.BtnA.wasPressed()) {
        dismissScene();
        return;
    }
}
void Scene::display() {}

void Scene::setNeedsDisplay() {
    this->needsDisplay = true;
}
bool Scene::displayIfNeeded() {
    if (this->needsDisplay) {
        this->display();
        this->needsDisplay = false;
        return true;
    }
    return false;
}

void Scene::clearButton(int index, int width, int color) {
    int displayWidth = M5.Display.width(), displayHeight = M5.Display.height();
    if (M5.Display.getRotation() == 2) {
        M5.Display.fillRect(0, displayHeight - 120, displayWidth, 120, color);
    } else {
        M5.Display.fillRect(0, displayHeight - 40, displayWidth, 40, color);
    }
}

void Scene::drawButton(int index, int size, string text, int color, int backgroundColor) {
    M5.Display.setTextColor(color, backgroundColor);
    M5.Display.setFont(&lgfxJapanGothicP_24);
    int displayWidth = M5.Display.width(), displayHeight = M5.Display.height();

    static constexpr auto buttonRect = [](uint8_t rotation, int index, int displayWidth, int displayHeight, uint16_t *rect) {
        if (rotation != 2) {
            int margin = 2, unit = (displayWidth - margin) / 3;
            rect[0] = margin + unit * index;
            rect[1] = displayHeight - 40 + margin;
            rect[2] = unit - margin;
            rect[3] = 40 - margin * 2;
        } else {
            int margin = 2, unit = 40;
            rect[0] = margin;
            rect[1] = displayHeight - unit * (index + 1) + margin;
            rect[2] = displayWidth - margin * 2;
            rect[3] = unit - margin;
        }
    };

    uint16_t rect[4], addRect[4];
    uint8_t rotation = M5.Display.getRotation();
    buttonRect(rotation, index, displayWidth, displayHeight, rect);
    buttonRect(rotation, index + size - 1, displayWidth, displayHeight, addRect);
    uint16_t minX = std::min(rect[0], addRect[0]), minY = std::min(rect[1], addRect[1]);
    uint16_t maxX = std::max(rect[0] + rect[2], addRect[0] + addRect[2]), maxY = std::max(rect[1] + rect[3], addRect[1] + addRect[3]);
    rect[0] = minX;
    rect[1] = minY;
    rect[2] = maxX - minX;
    rect[3] = maxY - minY;

    M5.Display.fillRect(rect[0] + 1, rect[1] + 1, rect[2] - 2, rect[3] - 2, backgroundColor);
    M5.Display.drawRoundRect(rect[0], rect[1], rect[2], rect[3], 2, color);
    M5.Display.drawCenterString(text.c_str(), rect[0] + rect[2] / 2, rect[1] + (rect[3] - 24) / 2);
}

void Scene::presentScene(shared_ptr<Scene> scene) {
    if (!isUITask()) {
        uiTask->send([this, scene]() {
            this->presentScene(scene);
        });
        return;
    }

    this->willDisappear();
    if (!scene->loaded) {
        scene->didLoad();
        scene->loaded = true;
    }
    scene->willAppear();
    this->childScene = scene;
    scene->parentScene = weak_from_this();
    activeScene = scene;
    drawActiveScene(true);
    this->didDisappear();
    scene->didAppear();
}
void Scene::dismissScene() {
    auto currentScene = shared_from_this();
    if (!isUITask()) {
        uiTask->send([currentScene]() {
            currentScene->dismissScene();
        });
        return;
    }

    if (!currentScene->parentScene.has_value()) return;
    if (std::shared_ptr<Scene> parent = currentScene->parentScene.value().lock()) {
        currentScene->willDisappear();
        parent->willAppear();
        activeScene = parent;
        parent->childScene = nullptr;
        currentScene->parentScene = std::nullopt;
        drawActiveScene(true);
        currentScene->didDisappear();
        activeScene->didAppear();
    }
}
void Scene::swapScene(shared_ptr<Scene> scene) {
    if (!isUITask()) {
        uiTask->send([this, scene]() {
            this->swapScene(scene);
        });
        return;
    }

    this->willDisappear();
    if (!scene->loaded) {
        scene->didLoad();
        scene->loaded = true;
    }
    scene->willAppear();

    if (this->parentScene.has_value()) {
        auto parent = this->parentScene.value().lock();
        parent->childScene = scene;
        scene->parentScene = this->parentScene;
        activeScene = scene;
        this->parentScene = std::nullopt;
    } else {
        rootScene = scene;
        activeScene = scene;
    }
    drawActiveScene(true);
    this->didDisappear();
    scene->didAppear();
}


UI_IMPL_END
