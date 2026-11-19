#include "RatePopup.hpp"

using namespace geode::prelude;

namespace {
    std::function<void(bool, bool)> g_rateCallback;
}

// Este hook detecta la respuesta del servidor al like/dislike.
// Se queda aquí mismo porque solo lo usa este popup.
#include <Geode/modify/GameLevelManager.hpp>
class $modify(RateHook, GameLevelManager) {
    void onLikeItemCompleted(gd::string response, gd::string tag) {
        GameLevelManager::onLikeItemCompleted(response, tag);
        std::string r = response;
        bool ok = (r == "1");
        bool already = (r == "-1"); // ya habías votado este nivel con esta cuenta
        if (g_rateCallback) g_rateCallback(ok, already);
    }
};

RatePopup* RatePopup::create() {
    auto ret = new RatePopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

RatePopup::~RatePopup() {
    g_rateCallback = nullptr;
}

bool RatePopup::init() {
    if (!Popup::init(300.f, 230.f)) return false;
    this->setTitle("Like / Dislike");

    auto size = m_mainLayer->getContentSize();

    auto lvlLbl = CCLabelBMFont::create("ID del nivel", "bigFont.fnt");
    lvlLbl->setScale(0.4f);
    lvlLbl->setPosition({size.width / 2, size.height - 50.f});
    m_mainLayer->addChild(lvlLbl);

    m_levelInput = TextInput::create(140.f, "ID", "bigFont.fnt");
    m_levelInput->setCommonFilter(CommonFilter::Uint);
    m_levelInput->setMaxCharCount(9);
    m_levelInput->setPosition({size.width / 2, size.height - 80.f});
    m_mainLayer->addChild(m_levelInput);

    auto cntLbl = CCLabelBMFont::create("Cantidad (max 50000)", "bigFont.fnt");
    cntLbl->setScale(0.4f);
    cntLbl->setPosition({size.width / 2, size.height - 110.f});
    m_mainLayer->addChild(cntLbl);

    m_countInput = TextInput::create(140.f, "Cantidad", "bigFont.fnt");
    m_countInput->setCommonFilter(CommonFilter::Uint);
    m_countInput->setMaxCharCount(5);
    m_countInput->setString("1");
    m_countInput->setPosition({size.width / 2, size.height - 140.f});
    m_mainLayer->addChild(m_countInput);

    m_status = CCLabelBMFont::create("Listo", "chatFont.fnt");
    m_status->setPosition({size.width / 2, size.height - 170.f});
    m_mainLayer->addChild(m_status);

    auto like = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Like"), this, menu_selector(RatePopup::onLike));
    auto dislike = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Dislike"), this, menu_selector(RatePopup::onDislike));
    auto stop = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Parar"), this, menu_selector(RatePopup::onStop));
    like->setPosition({size.width / 2 - 90.f, 30.f});
    dislike->setPosition({size.width / 2, 30.f});
    stop->setPosition({size.width / 2 + 90.f, 30.f});
    m_buttonMenu->addChild(like);
    m_buttonMenu->addChild(dislike);
    m_buttonMenu->addChild(stop);

    return true;
}

void RatePopup::refresh() {
    const char* tipo = m_isLike ? "likes" : "dislikes";
    m_status->setString(fmt::format(
        "{} / {} {} | {} ya votados", m_ok, m_total, tipo, m_skip).c_str());
}

void RatePopup::onLike(CCObject*) { this->begin(true); }
void RatePopup::onDislike(CCObject*) { this->begin(false); }

void RatePopup::begin(bool like) {
    if (m_running) return;

    m_levelID = geode::utils::numFromString<int>(m_levelInput->getString()).unwrapOr(0);
    if (m_levelID <= 0) {
        FLAlertLayer::create("Like / Dislike", "Pon un ID de nivel valido.", "OK")->show();
        return;
    }

    int n = geode::utils::numFromString<int>(m_countInput->getString()).unwrapOr(1);
    if (n < 1) n = 1;
    if (n > 50000) n = 50000;

    m_isLike = like;
    m_total = n;
    m_sent = m_ok = m_skip = 0;
    m_waiting = false;
    m_waitTime = 0.f;
    m_cooldown = 0.f;
    m_running = true;

    g_rateCallback = [this](bool ok, bool already) {
        if (m_running && m_waiting) this->handleResult(ok, already);
    };

    refresh();
    this->scheduleUpdate();
}

void RatePopup::onStop(CCObject*) {
    if (!m_running) return;
    m_running = false;
    m_waiting = false;
    this->unscheduleUpdate();
    g_rateCallback = nullptr;
    refresh();
}

void RatePopup::handleResult(bool ok, bool already) {
    m_waiting = false;
    if (ok) m_ok++;
    else if (already) m_skip++;
    m_sent++;
    m_cooldown = static_cast<float>(Mod::get()->getSettingValue<double>("rate-pause"));
    refresh();
}

void RatePopup::sendOne() {
    m_waiting = true;
    m_waitTime = 0.f;
    GameLevelManager::sharedState()->likeItem(LikeItemType::Level, m_levelID, m_isLike, 0);
}

void RatePopup::update(float dt) {
    if (!m_running) return;

    if (m_cooldown > 0.f) { m_cooldown -= dt; return; }

    if (m_waiting) {
        m_waitTime += dt;
        if (m_waitTime > 15.f) this->handleResult(false, false); // sin respuesta
        return;
    }

    if (m_sent >= m_total) {
        m_running = false;
        this->unscheduleUpdate();
        g_rateCallback = nullptr;
        return;
    }

    this->sendOne();
}
