#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "FloatingButton.hpp"

using namespace geode::prelude;

class $modify(LBMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        this->addChild(FloatingButton::create(), 100);
        return true;
    }
};
