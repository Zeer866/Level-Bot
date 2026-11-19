#include "FloatingButton.hpp"
#include "HubPopup.hpp"
#include <filesystem>
#include <algorithm>

using namespace geode::prelude;

FloatingButton* FloatingButton::s_instance = nullptr;

namespace {
    float g_iconBaseScale = 1.f; // escala del ícono tal como se crea, sin multiplicador
}

FloatingButton* FloatingButton::create() {
    auto ret = new FloatingButton();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

FloatingButton::~FloatingButton() {
    if (s_instance == this) s_instance = nullptr;
}

bool FloatingButton::init() {
    if (!CCLayer::init()) return false;

    auto win = CCDirector::sharedDirector()->getWinSize();

    CCNode* icon = nullptr;
    auto path = Mod::get()->getResourcesDir() / "botlogo.png";
    std::error_code ec;
    if (std::filesystem::exists(path, ec)) {
        if (auto spr = CCSprite::create(path.string().c_str())) {
            spr->setScale(0.19f);
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
    g_iconBaseScale = icon->getScale(); // guarda la escala original, antes de cualquier multiplicador

    float scaleMult = Mod::get()->getSavedValue<float>("btn-scale-mult", 1.f);
    icon->setScale(g_iconBaseScale * scaleMult);
    m_baseScale = icon->getScale();

    float fx = Mod::get()->getSavedValue<float>("btn-fx", 0.06f);
    float fy = Mod::get()->getSavedValue<float>("btn-fy", 0.62f);
    icon->setPosition({win.width * fx, win.height * fy});
    this->addChild(icon);

    this->setTouchEnabled(true);
    s_instance = this;
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

void FloatingButton::setTint(int r, int g, int b) {
    if (!s_instance || !s_instance->m_icon) return;
    if (auto rgba = dynamic_cast<CCRGBAProtocol*>(s_instance->m_icon)) {
        rgba->setColor({
            static_cast<GLubyte>(std::clamp(r, 0, 255)),
            static_cast<GLubyte>(std::clamp(g, 0, 255)),
            static_cast<GLubyte>(std::clamp(b, 0, 255))
        });
    }
}

void FloatingButton::setButtonScale(float s) {
    if (!s_instance || !s_instance->m_icon) return;
    s = std::clamp(s, 0.3f, 3.f);

    float newScale = g_iconBaseScale * s;
    s_instance->m_icon->setScale(newScale);
    s_instance->m_baseScale = newScale;

    Mod::get()->setSavedValue<float>("btn-scale-mult", s);
}

void FloatingButton::setButtonPos(float fx, float fy) {
    if (!s_instance || !s_instance->m_icon) return;
    fx = std::clamp(fx, 0.f, 1.f);
    fy = std::clamp(fy, 0.f, 1.f);
    auto win = CCDirector::sharedDirector()->getWinSize();
    s_instance->m_icon->setPosition({win.width * fx, win.height * fy});
    Mod::get()->setSavedValue<float>("btn-fx", fx);
    Mod::get()->setSavedValue<float>("btn-fy", fy);
}

void FloatingButton::resetButton() {
    Mod::get()->setSavedValue<float>("btn-fx", 0.06f);
    Mod::get()->setSavedValue<float>("btn-fy", 0.62f);
    Mod::get()->setSavedValue<float>("btn-scale-mult", 1.f);

    if (s_instance && s_instance->m_icon) {
        auto win = CCDirector::sharedDirector()->getWinSize();
        if (auto rgba = dynamic_cast<CCRGBAProtocol*>(s_instance->m_icon)) {
            rgba->setColor({255, 255, 255});
        }
        s_instance->m_icon->setScale(g_iconBaseScale);
        s_instance->m_baseScale = g_iconBaseScale;
        s_instance->m_icon->setPosition({win.width * 0.06f, win.height * 0.62f});
    }
}
