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
    if (!Popup::init(300.f, 290.f)) return false;
    this->setTitle("Editor de stats");

    struct Row { const char* label; const char* key; };
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
    GameLevelManager::sharedState()->updateUserScore();
    FLAlertLayer::create("Stats", "Stats aplicadas y enviadas al servidor.", "OK")->show();
}
