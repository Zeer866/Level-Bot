#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "HubPopup.hpp"

using namespace geode::prelude;

class $modify(LBMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        CCNode* icon = CCSprite::create("botlogo.png"_spr);
        if (icon) icon->setScale(0.15f); // el logo mide 336 px
        else icon = ButtonSprite::create("Bot");

        auto btn = CCMenuItemExt::createSpriteExtra(icon, [](CCObject*) {
            HubPopup::create()->show();
        });
        btn->setID("levelbot-button"_spr);

        if (auto menu = this->getChildByID("bottom-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        }
        return true;
    }
};
