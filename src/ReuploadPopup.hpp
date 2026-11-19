#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ReuploadPopup : public geode::Popup {
protected:
    geode::TextInput* m_idInput = nullptr;
    CCLabelBMFont* m_status = nullptr;
    int m_targetID = 0;
    bool m_waitingDownload = false;
    float m_waitTime = 0.f;

    bool init();
    void onGo(CCObject*);
    void update(float dt) override;
    void finishDownload(bool ok);

public:
    ~ReuploadPopup();
    static ReuploadPopup* create();
};
