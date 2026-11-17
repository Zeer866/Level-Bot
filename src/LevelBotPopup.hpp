#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LevelBotPopup : public geode::Popup {
protected:
    geode::TextInput* m_input = nullptr;
    CCLabelBMFont* m_status = nullptr;
    int m_total = 0;
    int m_done = 0;
    int m_failed = 0;
    bool m_running = false;

    bool init();
    void onStart(CCObject*);
    void onStop(CCObject*);
    void tick(float dt);
    void refresh();

public:
    static LevelBotPopup* create();
};
