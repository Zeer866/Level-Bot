#pragma once
#include <Geode/Geode.hpp>
#include "ScriptEngine.hpp"

using namespace geode::prelude;

class ScriptPopup : public geode::Popup {
protected:
    geode::TextInput* m_input = nullptr;
    CCLabelBMFont* m_status = nullptr;
    zs::Engine m_engine;
    bool m_running = false;

    bool init();
    void onRun(CCObject*);
    void onStop(CCObject*);
    void onExample(CCObject*);
    void update(float dt) override;
    void finish(const std::string& msg);

public:
    static ScriptPopup* create();
};
