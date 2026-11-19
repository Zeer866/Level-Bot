#pragma once
#include <Geode/Geode.hpp>
#include <vector>

using namespace geode::prelude;

class StatsPopup : public geode::Popup {
protected:
    std::vector<std::pair<const char*, geode::TextInput*>> m_fields;

    bool init();
    void onApply(CCObject*);

public:
    static StatsPopup* create();
};
