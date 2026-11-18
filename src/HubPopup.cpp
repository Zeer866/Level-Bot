#include "HubPopup.hpp"
#include "LevelBotPopup.hpp"
#include "AccountBotPopup.hpp"
#include "StatsPopup.hpp"
#include "ScriptPopup.hpp"

using namespace geode::prelude;

HubPopup* HubPopup::create() {
    auto ret = new HubPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool HubPopup::init() {
    if (!Popup::init(360.f, 250.f, "GJ_square02.png")) return false;
    this->setTitle("Level Bot");

    auto size = m_mainLayer->getContentSize();

    auto sub = CCLabelBMFont::create("Herramientas para tu GDPS", "chatFont.fnt");
    sub->setScale(0.8f);
    sub->setOpacity(180);
    sub->setPosition({size.width / 2, size.height - 48.f});
    m_mainLayer->addChild(sub);

    auto addCard = [&](const char* title, const char* desc, const char* tex,
                       float x, float y, std::function<void()> action) {
        auto spr = ButtonSprite::create(title, 140, true, "goldFont.fnt", tex, 42.f, 0.8f);
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [action](CCObject*) { action(); });
        btn->setPosition({x, y});
        m_buttonMenu->addChild(btn);

        auto lbl = CCLabelBMFont::create(desc, "chatFont.fnt");
        lbl->setScale(0.75f);
        lbl->setOpacity(190);
        lbl->setPosition({x, y - 33.f});
        m_mainLayer->addChild(lbl);
    };

    float cx = size.width / 2;
    float top = size.height - 100.f;
    float row = 85.f;

    addCard("Niveles", "Genera y sube niveles", "GJ_button_01.png", cx - 85.f, top,
        [] { LevelBotPopup::create()->show(); });
    addCard("Cuentas", "Crea cuentas de prueba", "GJ_button_02.png", cx + 85.f, top,
        [] { AccountBotPopup::create()->show(); });
    addCard("Stats", "Edita estrellas, lunas...", "GJ_button_03.png", cx - 85.f, top - row,
        [] { StatsPopup::create()->show(); });
    addCard("Scripts", "Programa con ZeerScript", "GJ_button_04.png", cx + 85.f, top - row,
        [] { ScriptPopup::create()->show(); });

    auto ver = CCLabelBMFont::create("v1.3.0 - Zeer866", "chatFont.fnt");
    ver->setScale(0.6f);
    ver->setOpacity(120);
    ver->setPosition({size.width / 2, 14.f});
    m_mainLayer->addChild(ver);

    return true;
}
