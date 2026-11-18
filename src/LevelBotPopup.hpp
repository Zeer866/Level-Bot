#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AccountBotPopup : public geode::Popup {
protected:
    geode::TextInput* m_input = nullptr;
    CCLabelBMFont* m_status = nullptr;
    int m_total = 0;
    int m_index = 0;
    int m_ok = 0;
    int m_skip = 0;
    int m_fail = 0;
    bool m_running = false;
    bool m_waiting = false;
    float m_waitTime = 0.f;
    float m_cooldown = 0.f;

    bool init();
    void onStart(CCObject*);
    void onStop(CCObject*);
    void begin();
    void update(float dt) override;
    void sendOne();
    void handleResult(const std::string& response);
    void refresh();

public:
    ~AccountBotPopup();
    static AccountBotPopup* create();
};
