#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

class AccountBotPopup : public geode::Popup {
protected:
    geode::TextInput* m_input = nullptr;
    CCLabelBMFont* m_status = nullptr;
    TaskHolder<web::WebResponse> m_holder;
    int m_total = 0;
    int m_index = 0;
    int m_ok = 0;
    int m_skip = 0;
    int m_fail = 0;
    bool m_running = false;
    bool m_waiting = false;
    float m_cooldown = 0.f;

    bool init();
    void onStart(CCObject*);
    void onStop(CCObject*);
    void update(float dt) override;
    void sendOne();
    void refresh();

public:
    static AccountBotPopup* create();
};
