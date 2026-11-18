#include "LevelBotPopup.hpp"
#include "Uploader.hpp"
#include "Generator.hpp"
#include "State.hpp"
#include <random>
#include <chrono>

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
    if (m_hasNext && m_next.valid()) m_next.wait();
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

void LevelBotPopup::prepareNext() {
    if (m_hasNext) return;
    int objects = Mod::get()->getSettingValue<int64_t>("objects");
    unsigned int seed = std::random_device{}();
    m_nextObjects = objects;
    m_next = std::async(std::launch::async, [seed, objects]() {
        return lb::generateLevel(seed, objects);
    });
    m_hasNext = true;
}

void LevelBotPopup::onStart(CCObject*) {
    if (m_running) return;
    int n = geode::utils::numFromString<int>(m_input->getString()).unwrapOr(0);
    if (n < 1) n = 1;
    if (n > 1000) n = 1000;

    if (m_hasNext && m_next.valid()) m_next.wait();
    m_hasNext = false;
    m_current.clear();

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

    this->prepareNext(); // el primero ya se va generando
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
        m_current.clear();
        m_cooldown = pause;
    } else {
        m_attempt++;
        if (m_attempt >= 6) {
            m_failed++;
            m_attempt = 0;
            m_current.clear();
            m_cooldown = pause;
        } else {
            // se reintenta el MISMO nivel (m_current se conserva)
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
        FMODAudioEngine::sharedEngine()->playEffect("achievement_01.ogg");
        return;
    }

    // Si no es un reintento, toma el nivel que se preparó en segundo plano
    if (m_current.empty()) {
        this->prepareNext();
        if (m_next.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
        m_current = m_next.get();
        m_hasNext = false;
    }

    int index = m_done + m_failed + 1;
    if (lb::uploadGeneratedLevel(index, m_current, m_nextObjects)) {
        m_waiting = true;
        m_waitTime = 0.f;
        // mientras el servidor responde, ya se genera el siguiente
        if (m_done + m_failed + 1 < m_total) this->prepareNext();
    } else {
        FLAlertLayer::create("Level Bot", "Inicia sesion en tu cuenta del GDPS primero.", "OK")->show();
        this->onStop(nullptr);
    }
}
