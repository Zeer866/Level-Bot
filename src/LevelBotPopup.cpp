#include "LevelBotPopup.hpp"
#include "Uploader.hpp"
#include "State.hpp"
#include <random>

using namespace geode::prelude;

LevelBotPopup* LevelBotPopup::create() {
    auto ret = new LevelBotPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

LevelBotPopup::~LevelBotPopup() {
    lb::clearUploadCallback();
}

bool LevelBotPopup::init() {
    if (!Popup::init(300.f, 210.f)) return false;
    this->setTitle("Level Bot");

    auto size = m_mainLayer->getContentSize();

    auto info = CCLabelBMFont::create("Cuantos niveles? (1 - 1000)", "bigFont.fnt");
    info->setScale(0.45f);
    info->setPosition({size.width / 2, size.height - 55.f});
    m_mainLayer->addChild(info);

    m_input = TextInput::create(140.f, "Cantidad", "bigFont.fnt");
    m_input->setCommonFilter(CommonFilter::Uint);
    m_input->setMaxCharCount(4);
    m_input->setString("10");
    m_input->setPosition({size.width / 2, size.height - 90.f});
    m_mainLayer->addChild(m_input);

    m_status = CCLabelBMFont::create("Listo", "chatFont.fnt");
    m_status->setPosition({size.width / 2, size.height - 125.f});
    m_mainLayer->addChild(m_status);

    auto start = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Iniciar"), this, menu_selector(LevelBotPopup::onStart));
    auto stop = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Parar"), this, menu_selector(LevelBotPopup::onStop));
    start->setPosition({size.width / 2 - 60.f, 35.f});
    stop->setPosition({size.width / 2 + 60.f, 35.f});
    m_buttonMenu->addChild(start);
    m_buttonMenu->addChild(stop);

    return true;
}

void LevelBotPopup::refresh() {
    std::string text = fmt::format(
        "{} / {} subidos  |  {} fallidos", m_done, m_total, m_failed);
    if (m_attempt > 0) text += fmt::format("  |  reintento {}", m_attempt);
    m_status->setString(text.c_str());
}

void LevelBotPopup::onStart(CCObject*) {
    if (m_running) return;
    int n = geode::utils::numFromString<int>(m_input->getString()).unwrapOr(0);
    if (n < 1) n = 1;
    if (n > 1000) n = 1000;

    m_total = n;
    m_done = 0;
    m_failed = 0;
    m_attempt = 0;
    m_waiting = false;
    m_waitTime = 0.f;
    m_cooldown = 0.f;
    m_running = true;

    lb::setUploadCallback([this](bool ok) {
        if (m_running && m_waiting) this->handleResult(ok);
    });

    refresh();
    this->scheduleUpdate();
}

void LevelBotPopup::onStop(CCObject*) {
    if (!m_running) return;
    m_running = false;
    m_waiting = false;
    this->unscheduleUpdate();
    lb::clearUploadCallback();
    m_status->setString(fmt::format("Detenido ({} / {})", m_done, m_total).c_str());
}

void LevelBotPopup::handleResult(bool ok) {
    m_waiting = false;
    float pause = static_cast<float>(Mod::get()->getSettingValue<double>("pause"));

    if (ok) {
        m_done++;
        m_attempt = 0;
        m_cooldown = pause;
    } else {
        m_attempt++;
        if (m_attempt >= 6) {
            // se rindió con este nivel, pasa al siguiente
            m_failed++;
            m_attempt = 0;
            m_cooldown = pause;
        } else {
            // espera creciente antes de reintentar el MISMO nivel
            m_cooldown = std::min(3.f * m_attempt, 15.f);
        }
    }
    refresh();
}

void LevelBotPopup::update(float dt) {
    if (!m_running) return;

    if (m_cooldown > 0.f) {
        m_cooldown -= dt;
        return;
    }

    if (m_waiting) {
        m_waitTime += dt;
        if (m_waitTime > 30.f) this->handleResult(false); // sin respuesta
        return;
    }

    if (m_done + m_failed >= m_total) {
        m_running = false;
        this->unscheduleUpdate();
        lb::clearUploadCallback();
        m_status->setString(fmt::format(
            "Terminado: {} subidos, {} fallidos", m_done, m_failed).c_str());
        return;
    }

    int objects = Mod::get()->getSettingValue<int64_t>("objects");
    unsigned int seed = std::random_device{}();
    int index = m_done + m_failed + 1; // no cambia mientras se reintenta

    if (lb::uploadGeneratedLevel(index, seed, objects)) {
        m_waiting = true;
        m_waitTime = 0.f;
    } else {
        FLAlertLayer::create("Level Bot", "Inicia sesion en tu cuenta del GDPS primero.", "OK")->show();
        this->onStop(nullptr);
    }
}
