#include "ScriptPopup.hpp"

using namespace geode::prelude;

ScriptPopup* ScriptPopup::create() {
    auto ret = new ScriptPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ScriptPopup::init() {
    if (!Popup::init(340.f, 220.f)) return false;
    this->setTitle("ZeerScript");

    auto size = m_mainLayer->getContentSize();

    auto info = CCLabelBMFont::create("Separa los comandos con ;", "chatFont.fnt");
    info->setPosition({size.width / 2, size.height - 50.f});
    m_mainLayer->addChild(info);

    m_input = TextInput::create(300.f, "repetir 3; decir hola; esperar 1; fin", "chatFont.fnt");
    m_input->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ;$#_.,-!?");
    m_input->setMaxCharCount(600);
    m_input->setPosition({size.width / 2, size.height - 85.f});
    m_mainLayer->addChild(m_input);

    m_status = CCLabelBMFont::create("Listo", "chatFont.fnt");
    m_status->setPosition({size.width / 2, size.height - 125.f});
    m_mainLayer->addChild(m_status);

    auto run = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Ejecutar"), this, menu_selector(ScriptPopup::onRun));
    auto stop = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Parar"), this, menu_selector(ScriptPopup::onStop));
    auto example = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Ejemplo"), this, menu_selector(ScriptPopup::onExample));
    run->setPosition({size.width / 2 - 100.f, 35.f});
    stop->setPosition({size.width / 2, 35.f});
    example->setPosition({size.width / 2 + 100.f, 35.f});
    m_buttonMenu->addChild(run);
    m_buttonMenu->addChild(stop);
    m_buttonMenu->addChild(example);

    return true;
}

void ScriptPopup::onExample(CCObject*) {
    m_input->setString(
        "poner n 0; repetir 3; sumar n 1; si $n >= 2; decir ya casi; sino; decir faltan; fin; fin; "
        "nivel 5; decir listo; sonido");
}

void ScriptPopup::finish(const std::string& msg) {
    m_running = false;
    this->unscheduleUpdate();
    m_status->setString(msg.c_str());
}

void ScriptPopup::onRun(CCObject*) {
    if (m_running) return;
    std::string error;
    if (!m_engine.load(m_input->getString(), error)) {
        m_status->setString(error.c_str());
        return;
    }
    m_running = true;
    m_status->setString("Ejecutando...");
    this->scheduleUpdate();
}

void ScriptPopup::onStop(CCObject*) {
    if (m_running) this->finish("Detenido");
}

void ScriptPopup::update(float dt) {
    if (!m_running) return;

    std::string error;
    bool alive = m_engine.step(dt, [this](const std::string& s) {
        m_status->setString(s.c_str());
    }, error);

    if (!error.empty()) {
        this->finish(error);
    } else if (!alive) {
        m_running = false;
        this->unscheduleUpdate();
    }
}
