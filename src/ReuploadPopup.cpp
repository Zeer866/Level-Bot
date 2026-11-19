#include "ReuploadPopup.hpp"

using namespace geode::prelude;

namespace {
    std::function<void(bool)> g_reuploadCallback;
}

#include <Geode/modify/GameLevelManager.hpp>
class $modify(ReuploadHook, GameLevelManager) {
    void onDownloadLevelCompleted(gd::string response, gd::string tag) {
        GameLevelManager::onDownloadLevelCompleted(response, tag);
        bool ok = !std::string(response).empty() && std::string(response) != "-1";
        if (g_reuploadCallback) g_reuploadCallback(ok);
    }
};

ReuploadPopup* ReuploadPopup::create() {
    auto ret = new ReuploadPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

ReuploadPopup::~ReuploadPopup() {
    g_reuploadCallback = nullptr;
}

bool ReuploadPopup::init() {
    if (!Popup::init(300.f, 180.f)) return false;
    this->setTitle("Resubir nivel");

    auto size = m_mainLayer->getContentSize();

    auto info = CCLabelBMFont::create("ID del nivel de GD a copiar", "bigFont.fnt");
    info->setScale(0.4f);
    info->setPosition({size.width / 2, size.height - 50.f});
    m_mainLayer->addChild(info);

    m_idInput = TextInput::create(140.f, "ID", "bigFont.fnt");
    m_idInput->setCommonFilter(CommonFilter::Uint);
    m_idInput->setMaxCharCount(9);
    m_idInput->setPosition({size.width / 2, size.height - 85.f});
    m_mainLayer->addChild(m_idInput);

    m_status = CCLabelBMFont::create("Listo", "chatFont.fnt");
    m_status->setPosition({size.width / 2, size.height - 120.f});
    m_mainLayer->addChild(m_status);

    auto go = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Resubir"), this, menu_selector(ReuploadPopup::onGo));
    go->setPosition({size.width / 2, 30.f});
    m_buttonMenu->addChild(go);

    return true;
}

void ReuploadPopup::onGo(CCObject*) {
    if (m_waitingDownload) return;

    m_targetID = geode::utils::numFromString<int>(m_idInput->getString()).unwrapOr(0);
    if (m_targetID <= 0) {
        FLAlertLayer::create("Resubir", "Pon un ID de nivel valido.", "OK")->show();
        return;
    }
    auto am = GJAccountManager::sharedState();
    if (am->m_accountID <= 0) {
        FLAlertLayer::create("Resubir", "Inicia sesion en tu cuenta del GDPS primero.", "OK")->show();
        return;
    }

    g_reuploadCallback = [this](bool ok) { this->finishDownload(ok); };
    m_waitingDownload = true;
    m_waitTime = 0.f;
    m_status->setString("Descargando...");

    GameLevelManager::sharedState()->downloadLevel(m_targetID, false, 0);
    this->scheduleUpdate();
}

void ReuploadPopup::update(float dt) {
    if (!m_waitingDownload) return;
    m_waitTime += dt;
    if (m_waitTime > 15.f) this->finishDownload(false);
}

void ReuploadPopup::finishDownload(bool ok) {
    m_waitingDownload = false;
    this->unscheduleUpdate();
    g_reuploadCallback = nullptr;

    if (!ok) {
        m_status->setString("No se pudo descargar ese nivel.");
        return;
    }

    auto original = GameLevelManager::sharedState()->getSavedLevel(m_targetID);
    if (!original) {
        m_status->setString("Se descargo pero no se encontro el nivel.");
        return;
    }

    auto copy = GJGameLevel::create();
    copy->m_levelName = original->m_levelName;
    copy->m_levelDesc = original->m_levelDesc;
    copy->m_levelString = original->m_levelString;
    copy->m_levelType = GJLevelType::Editor;
    copy->m_levelVersion = 1;
    copy->m_audioTrack = original->m_audioTrack;
    copy->m_songID = original->m_songID;
    copy->m_objectCount = original->m_objectCount;
    copy->m_unlisted = false;

    GameLevelManager::sharedState()->uploadLevel(copy);
    m_status->setString("Subiendo copia a tu GDPS...");
}
