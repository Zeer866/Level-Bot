#include "AccountBotPopup.hpp"
#include "State.hpp"
#include <algorithm>

using namespace geode::prelude;

AccountBotPopup* AccountBotPopup::create() {
    auto ret = new AccountBotPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

AccountBotPopup::~AccountBotPopup() {
    lb::clearAccountCallback();
}

bool AccountBotPopup::init() {
    if (!Popup::init(300.f, 210.f)) return false;
    this->setTitle("Bot de cuentas");

    auto size = m_mainLayer->getContentSize();

    auto info = CCLabelBMFont::create("Cuantas cuentas? (1 - 1000)", "bigFont.fnt");
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
        ButtonSprite::create("Iniciar"), this, menu_selector(AccountBotPopup::onStart));
    auto stop = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Parar"), this, menu_selector(AccountBotPopup::onStop));
    start->setPosition({size.width / 2 - 60.f, 35.f});
    stop->setPosition({size.width / 2 + 60.f, 35.f});
    m_buttonMenu->addChild(start);
    m_buttonMenu->addChild(stop);

    return true;
}

void AccountBotPopup::refresh() {
    m_status->setString(fmt::format(
        "{} / {} creadas | {} ya existian | {} fallidas",
        m_ok, m_total, m_skip, m_fail).c_str());
}

void AccountBotPopup::onStart(CCObject*) {
    if (m_running) return;

    // El bot usa el servidor al que apunta el juego (tu switcher), así que se confirma antes
    geode::createQuickPopup(
        "Bot de cuentas",
        "Se crearan cuentas en el servidor que tengas activo ahora.\n"
        "Confirma que es <cy>tu GDPS</c> y no el servidor oficial.",
        "Cancelar", "Iniciar",
        [this](FLAlertLayer*, bool confirmed) {
            if (confirmed) this->begin();
        }
    );
}

void AccountBotPopup::begin() {
    int n = geode::utils::numFromString<int>(m_input->getString()).unwrapOr(0);
    if (n < 1) n = 1;
    if (n > 1000) n = 1000;

    m_total = n;
    m_index = 0;
    m_ok = m_skip = m_fail = 0;
    m_waiting = false;
    m_waitTime = 0.f;
    m_cooldown = 0.f;
    m_running = true;

    lb::setAccountCallback([this](const std::string& r) {
        if (m_running && m_waiting) this->handleResult(r);
    });

    refresh();
    this->scheduleUpdate();
}

void AccountBotPopup::onStop(CCObject*) {
    if (!m_running) return;
    m_running = false;
    m_waiting = false;
    this->unscheduleUpdate();
    lb::clearAccountCallback();
    m_status->setString(fmt::format("Detenido ({} creadas)", m_ok).c_str());
}

void AccountBotPopup::handleResult(const std::string& response) {
    if (response == "1") m_ok++;
    else if (response == "-2" || response == "-3") m_skip++; // usuario o correo ya existe
    else m_fail++;

    m_index++;
    m_waiting = false;
    m_cooldown = static_cast<float>(Mod::get()->getSettingValue<double>("acc-pause"));
    refresh();
}

void AccountBotPopup::sendOne() {
    auto prefix = Mod::get()->getSettingValue<std::string>("acc-prefix");
    if (prefix.size() > 11) prefix.resize(11); // GD permite 15 caracteres de usuario
    auto pass = Mod::get()->getSettingValue<std::string>("acc-password");
    auto domain = Mod::get()->getSettingValue<std::string>("acc-email-domain");
    int start = Mod::get()->getSettingValue<int64_t>("acc-start");

    std::string name = prefix + std::to_string(start + m_index);
    std::string email = name + "@" + domain;
    std::transform(email.begin(), email.end(), email.begin(),
        [](unsigned char c) { return std::tolower(c); });

    m_waiting = true;
    m_waitTime = 0.f;
    GJAccountManager::sharedState()->registerAccount(name, pass, email);
}

void AccountBotPopup::update(float dt) {
    if (!m_running) return;

    if (m_cooldown > 0.f) {
        m_cooldown -= dt;
        return;
    }

    if (m_waiting) {
        m_waitTime += dt;
        if (m_waitTime > 20.f) this->handleResult("-1"); // sin respuesta
        return;
    }

    if (m_index >= m_total) {
        m_running = false;
        this->unscheduleUpdate();
        lb::clearAccountCallback();
        m_status->setString(fmt::format(
            "Terminado: {} creadas, {} ya existian, {} fallidas",
            m_ok, m_skip, m_fail).c_str());
        return;
    }

    this->sendOne();
}
