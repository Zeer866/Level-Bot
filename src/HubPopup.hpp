#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class HubPopup : public geode::Popup {
protected:
    bool init();

public:
    static HubPopup* create();
};
