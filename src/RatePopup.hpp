#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RatePopup : public geode::Popup {
protected:
    geode::TextInput* m_levelInput = nullptr;
    geode::TextInput* m_countInput = nullptr;
    CCLabelBMFont* m_status = nullptr;
    int m_levelID = 0;
    int m_total = 0;
    int m_sent = 0;
    int m_ok = 0;
    int m_skip = 0;
    bool m_isLike = true;
    bool m_running = false;
    bool m_waiting = false;
    float m_waitTime = 0.f;
    float m_cooldown = 0.f;

    bool init();
    void onLike(CCObject*);
    void onDislike(CCObject*);
    void onStop(CCObject*);
    void begin(bool like);
    void update(float dt) override;
    void sendOne();
    void handleResult(bool ok, bool already);
    void refresh();

public:
    ~RatePopup();
    static RatePopup* create();
};
