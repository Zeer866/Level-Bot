#include "FloatingButton.hpp"
#include "HubPopup.hpp"
#include <filesystem>

using namespace geode::prelude;

FloatingButton* FloatingButton::create() {
    auto ret = new FloatingButton();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool FloatingButton::init() {
    if (!CCLayer::init()) return false;

    auto win = CCDirector::sharedDirector()->getWinSize();

    // Intenta cargar tu logo; si no está en resources/, usa un botón verde redondo
    CCNode* icon = nullptr;
    auto path = Mod::get()->getResourcesDir() / "botlogo.png";
    std::error_code ec;
    if (std::filesystem::exists(path, ec)) {
        if (auto spr = CCSprite::create(path.string().c_str())) {
            spr->setScale(0.19f); // 336 px * 0.19 = unas 64 unidades
            icon = spr;
        }
    }
    if (!icon) {
        icon = CircleButtonSprite::create(
            CCLabelBMFont::create("Bot", "bigFont.fnt"),
            CircleBaseColor::Green,
            CircleBaseSize::Large
        );
    }

    m_icon = icon;
    m_baseScale = icon->getScale();

    float fx = Mod::get()->getSavedValue<float>("btn-fx", 0.06f);
    float fy = Mod::get()->getSavedValue<float>("btn-fy", 0.62f);
    icon->setPosition({win.width * fx, win.height * fy});
    this->addChild(icon);

    this->setTouchEnabled(true);
    return true;
}

void FloatingButton::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -500, true);
}

bool FloatingButton::ccTouchBegan(CCTouch* touch, CCEvent*) {
    if (!m_icon) return false;
    auto local = this->convertToNodeSpace(touch->getLocation());
    if (!m_icon->boundingBox().containsPoint(local)) return false;

    m_touchStart = touch->getLocation();
    m_startPos = m_icon->getPosition();
    m_dragging = false;

    m_icon->stopAllActions();
    m_icon->runAction(CCScaleTo::create(0.08f, m_baseScale * 0.9f));
    return true;
}

void FloatingButton::ccTouchMoved(CCTouch* touch, CCEvent*) {
    auto delta = touch->getLocation() - m_touchStart;
    if (!m_dragging && ccpLength(delta) > 12.f) m_dragging = true;
    if (!m_dragging) return;

    auto win = CCDirector::sharedDirector()->getWinSize();
    auto half = m_icon->getScaledContentSize() / 2;
    auto pos = m_startPos + delta;
    pos.x = std::clamp(pos.x, half.width, win.width - half.width);
    pos.y = std::clamp(pos.y, half.height, win.height - half.height);
    m_icon->setPosition(pos);
}

void FloatingButton::release() {
    m_icon->stopAllActions();
    m_icon->runAction(CCScaleTo::create(0.08f, m_baseScale));
}

void FloatingButton::ccTouchEnded(CCTouch*, CCEvent*) {
    this->release();

    if (m_dragging) {
        // guarda la posición como porcentaje de la pantalla
        auto win = CCDirector::sharedDirector()->getWinSize();
        auto pos = m_icon->getPosition();
        Mod::get()->setSavedValue<float>("btn-fx", pos.x / win.width);
        Mod::get()->setSavedValue<float>("btn-fy", pos.y / win.height);
    } else {
        HubPopup::create()->show();
    }
}

void FloatingButton::ccTouchCancelled(CCTouch*, CCEvent*) {
    this->release();
}
