#pragma once
#include <Geode/Geode.hpp>
#include <future>
#include <string>

using namespace geode::prelude;

class LevelBotPopup : public geode::Popup {
protected:
    geode::TextInput* m_input = nullptr;
    CCLabelBMFont* m_status = nullptr;
    int m_total = 0;
    int m_done = 0;
    int m_failed = 0;
    int m_attempt = 0;
    bool m_running = false;
    bool m_waiting = false;
    float m_waitTime = 0.f;
    float m_cooldown = 0.f;

    std::future<std::string> m_next;
    bool m_hasNext = false;
    int m_nextObjects = 0;
    std::string m_current;

    bool init();
    void onStart(CCObject*);
    void onStop(CCObject*);
    void update(float dt) override;
    void handleResult(bool ok);
    void prepareNext();
    void refresh();

public:
    ~LevelBotPopup();
    static LevelBotPopup* create();
};
