#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class FloatingButton : public CCLayer {
protected:
    CCNode* m_icon = nullptr;
    CCPoint m_touchStart;
    CCPoint m_startPos;
    float m_baseScale = 1.f;
    bool m_dragging = false;

    bool init() override;
    void registerWithTouchDispatcher() override;
    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;
    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override;
    void release();

public:
    static FloatingButton* create();
};
