#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "LevelBotPopup.hpp"
#include "StatsPopup.hpp"
#include "AccountBotPopup.hpp"

using namespace geode::prelude;

class $modify(LBMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto bot = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Bot"), [](CCObject*) {
            LevelBotPopup::create()->show();
        });
        bot->setID("levelbot-button"_spr);

        auto stats = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Stats"), [](CCObject*) {
            StatsPopup::create()->show();
        });
        stats->setID("levelbot-stats-button"_spr);

        auto accs = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Cuentas"), [](CCObject*) {
            AccountBotPopup::create()->show();
        });
        accs->setID("levelbot-accounts-button"_spr);

        if (auto menu = this->getChildByID("bottom-menu")) {
            menu->addChild(bot);
            menu->addChild(stats);
            menu->addChild(accs);
            menu->updateLayout();
        }
        return true;
    }
};
