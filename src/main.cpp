#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "LevelBotPopup.hpp"

using namespace geode::prelude;

class $modify(LBMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto spr = ButtonSprite::create("Bot");
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [](CCObject*) {
            LevelBotPopup::create()->show();
        });
        btn->setID("levelbot-button"_spr);

        if (auto menu = this->getChildByID("bottom-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        }
        return true;
    }
};
