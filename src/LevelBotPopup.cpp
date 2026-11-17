#include "LevelBotPopup.hpp"
#include "Uploader.hpp"
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

bool LevelBotPopup::init() {
    if (!Popup::init(300.f, 210.f)) return false;
    this->setTitle("Level Bot");

    auto size = m_mainLayer->getContentSize();

    auto info = CCLabelBMFont::create("Cuantos niveles? (1 - 1000)", "bigFont.fnt");
    info->setScale(0.45f);
    info->setPosition({size.width / 2, size.height - 55.f});
    m_mainLayer->addChild(info);

    m_input = TextInput::create(140.f, "Cantidad", "bigFont.fnt");
    m_input->setFilter(CommonFilter::Uint);
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
    m_status->setString(fmt::format(
        "{} / {} enviados  |  {} fallidos", m_done, m_total, m_failed).c_str());
}

void LevelBotPopup::onStart(CCObject*) {
    if (m_running) return;
    int n = geode::utils::numFromString<int>(m_input->getString()).unwrapOr(0);
    if (n < 1) n = 1;
    if (n > 1000) n = 1000;
    m_total = n;
    m_done = 0;
    m_failed = 0;
    m_running = true;
    refresh();

    int delay = Mod::get()->getSettingValue<int64_t>("delay");
    this->schedule(schedule_selector(LevelBotPopup::tick), static_cast<float>(delay));
    this->tick(0.f); // el primero sale ya
}

void LevelBotPopup::onStop(CCObject*) {
    if (!m_running) return;
    m_running = false;
    this->unschedule(schedule_selector(LevelBotPopup::tick));
    m_status->setString(fmt::format("Detenido ({} / {})", m_done, m_total).c_str());
}

void LevelBotPopup::tick(float) {
    if (!m_running) return;
    if (m_done + m_failed >= m_total) {
        m_running = false;
        this->unschedule(schedule_selector(LevelBotPopup::tick));
        return;
    }
    int objects = Mod::get()->getSettingValue<int64_t>("objects");
    unsigned int seed = std::random_device{}();
    int index = m_done + m_failed + 1;

    if (lb::uploadGeneratedLevel(index, seed, objects)) {
        m_done++;
    } else {
        m_failed++;
        FLAlertLayer::create("Level Bot", "Inicia sesion en tu cuenta del GDPS primero.", "OK")->show();
        this->onStop(nullptr);
    }
    refresh();
}
