#include "StatsPopup.hpp"

using namespace geode::prelude;

StatsPopup* StatsPopup::create() {
    auto ret = new StatsPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool StatsPopup::init() {
    if (!Popup::init(300.f, 320.f)) return false;
    this->setTitle("Editor de stats");

    struct Row { const char* label; const char* key; };
    // Claves de GameStatsManager (si alguna sale cambiada, se ajusta aquí)
    static const Row rows[] = {
        {"Estrellas", "6"},
        {"Diamantes", "13"},
        {"Lunas", "28"},
        {"Monedas usuario", "12"},
        {"Monedas secretas", "8"},
        {"Demonios", "5"},
    };

    auto gsm = GameStatsManager::sharedState();
    auto size = m_mainLayer->getContentSize();
    float y = size.height - 55.f;

    for (auto& r : rows) {
        auto lbl = CCLabelBMFont::create(r.label, "bigFont.fnt");
        lbl->setScale(0.4f);
        lbl->setAnchorPoint({0.f, 0.5f});
        lbl->setPosition({25.f, y});
        m_mainLayer->addChild(lbl);

        auto input = TextInput::create(110.f, "0", "bigFont.fnt");
        input->setCommonFilter(CommonFilter::Uint);
        input->setMaxCharCount(7);
        input->setString(std::to_string(gsm->getStat(r.key)));
        input->setPosition({size.width - 85.f, y});
        m_mainLayer->addChild(input);

        m_fields.push_back({r.key, input});
        y -= 33.f;
    }

    // --- toggle de moderador ---
    auto am = GJAccountManager::sharedState();
    bool isMod = am->m_isElder || am->m_isMod;

    auto onSpr = CCSprite::create("GJ_checkOn_001.png");
    auto offSpr = CCSprite::create("GJ_checkOff_001.png");
    m_modToggle = CCMenuItemToggler::create(offSpr, onSpr, this, nullptr);
    m_modToggle->toggle(isMod);
    m_modToggle->setPosition({size.width - 55.f, y - 15.f});
    m_buttonMenu->addChild(m_modToggle);

    auto modLabel = CCLabelBMFont::create("Moderador", "bigFont.fnt");
    modLabel->setScale(0.4f);
    modLabel->setAnchorPoint({0.f, 0.5f});
    modLabel->setPosition({25.f, y - 15.f});
    m_mainLayer->addChild(modLabel);

    auto apply = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Aplicar"), this, menu_selector(StatsPopup::onApply));
    apply->setPosition({size.width / 2, 30.f});
    m_buttonMenu->addChild(apply);

    return true;
}

void StatsPopup::onApply(CCObject*) {
    auto gsm = GameStatsManager::sharedState();
    for (auto& [key, input] : m_fields) {
        int v = geode::utils::numFromString<int>(input->getString()).unwrapOr(0);
        gsm->setStat(key, v);
    }

    auto am = GJAccountManager::sharedState();
    bool wantMod = m_modToggle->isOn();
    am->m_isMod = wantMod;
    am->m_isElder = wantMod;

    // Envía stats y bandera de mod al servidor
    GameLevelManager::sharedState()->updateUserScore();
    FLAlertLayer::create("Stats", "Stats y permisos aplicados y enviados al servidor.", "OK")->show();
}
