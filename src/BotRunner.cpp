#include "BotRunner.hpp"
#include "Generator.hpp"
#include "Uploader.hpp"
#include "State.hpp"
#include <Geode/Geode.hpp>
#include <random>
#include <algorithm>

using namespace geode::prelude;

namespace {
    // --- niveles ---
    bool g_lvlRunning = false;
    int g_lvlTotal = 0, g_lvlDone = 0, g_lvlFailed = 0, g_lvlAttempt = 0;
    bool g_lvlWaiting = false;
    float g_lvlWaitTime = 0.f, g_lvlCooldown = 0.f;
    std::string g_lvlCurrent;
    std::function<void(int, int)> g_lvlOnDone;

    void lvlFinish() {
        g_lvlRunning = false;
        lb::clearUploadCallback();
        if (g_lvlOnDone) g_lvlOnDone(g_lvlDone, g_lvlFailed);
    }

    // --- cuentas ---
    bool g_accRunning = false;
    int g_accTotal = 0, g_accIndex = 0, g_accOk = 0, g_accFail = 0;
    bool g_accWaiting = false;
    float g_accWaitTime = 0.f, g_accCooldown = 0.f;
    std::function<void(int, int)> g_accOnDone;

    void accFinish() {
        g_accRunning = false;
        lb::clearAccountCallback();
        if (g_accOnDone) g_accOnDone(g_accOk, g_accFail);
    }
}

void lb::runLevels(int count, std::function<void(int, int)> onDone) {
    g_lvlTotal = std::clamp(count, 1, 1000);
    g_lvlDone = g_lvlFailed = g_lvlAttempt = 0;
    g_lvlWaiting = false;
    g_lvlWaitTime = g_lvlCooldown = 0.f;
    g_lvlCurrent.clear();
    g_lvlOnDone = std::move(onDone);
    g_lvlRunning = true;

    lb::setUploadCallback([](bool ok) {
        if (!g_lvlRunning || !g_lvlWaiting) return;
        g_lvlWaiting = false;
        float pause = static_cast<float>(Mod::get()->getSettingValue<double>("pause"));
        if (ok) {
            g_lvlDone++;
            g_lvlAttempt = 0;
            g_lvlCurrent.clear();
            g_lvlCooldown = pause;
        } else {
            g_lvlAttempt++;
            if (g_lvlAttempt >= 6) {
                g_lvlFailed++;
                g_lvlAttempt = 0;
                g_lvlCurrent.clear();
                g_lvlCooldown = pause;
            } else {
                g_lvlCooldown = std::min(3.f * g_lvlAttempt, 15.f);
            }
        }
    });
}

bool lb::levelsRunning() { return g_lvlRunning; }

void lb::tickLevels(float dt) {
    if (!g_lvlRunning) return;

    if (g_lvlCooldown > 0.f) { g_lvlCooldown -= dt; return; }

    if (g_lvlWaiting) {
        g_lvlWaitTime += dt;
        if (g_lvlWaitTime > 30.f) {
            g_lvlWaiting = false;
            g_lvlAttempt++;
            g_lvlCooldown = 1.f;
        }
        return;
    }

    if (g_lvlDone + g_lvlFailed >= g_lvlTotal) {
        lvlFinish();
        return;
    }

    if (g_lvlCurrent.empty()) {
        int objects = Mod::get()->getSettingValue<int64_t>("objects");
        unsigned int seed = std::random_device{}();
        g_lvlCurrent = lb::generateLevel(seed, objects);
    }

    int objects = Mod::get()->getSettingValue<int64_t>("objects");
    int index = g_lvlDone + g_lvlFailed + 1;
    if (lb::uploadGeneratedLevel(index, g_lvlCurrent, objects)) {
        g_lvlWaiting = true;
        g_lvlWaitTime = 0.f;
    } else {
        lvlFinish(); // no hay sesión iniciada
    }
}

void lb::runAccounts(int count, std::function<void(int, int)> onDone) {
    g_accTotal = std::clamp(count, 1, 1000);
    g_accIndex = g_accOk = g_accFail = 0;
    g_accWaiting = false;
    g_accWaitTime = g_accCooldown = 0.f;
    g_accOnDone = std::move(onDone);
    g_accRunning = true;

    lb::setAccountCallback([](const std::string& response) {
        if (!g_accRunning || !g_accWaiting) return;
        g_accWaiting = false;
        if (response == "1") g_accOk++;
        else g_accFail++; // repetidas y errores cuentan como "no creada" aquí
        g_accIndex++;
        g_accCooldown = static_cast<float>(Mod::get()->getSettingValue<double>("acc-pause"));
    });
}

bool lb::accountsRunning() { return g_accRunning; }

void lb::tickAccounts(float dt) {
    if (!g_accRunning) return;

    if (g_accCooldown > 0.f) { g_accCooldown -= dt; return; }

    if (g_accWaiting) {
        g_accWaitTime += dt;
        if (g_accWaitTime > 20.f) {
            g_accWaiting = false;
            g_accFail++;
            g_accIndex++;
            g_accCooldown = 1.f;
        }
        return;
    }

    if (g_accIndex >= g_accTotal) {
        accFinish();
        return;
    }

    auto prefix = Mod::get()->getSettingValue<std::string>("acc-prefix");
    if (prefix.size() > 11) prefix.resize(11);
    auto pass = Mod::get()->getSettingValue<std::string>("acc-password");
    auto domain = Mod::get()->getSettingValue<std::string>("acc-email-domain");
    int start = Mod::get()->getSettingValue<int64_t>("acc-start");

    std::string name = prefix + std::to_string(start + g_accIndex);
    std::string email = name + "@" + domain;
    std::transform(email.begin(), email.end(), email.begin(),
        [](unsigned char c) { return std::tolower(c); });

    g_accWaiting = true;
    g_accWaitTime = 0.f;
    GJAccountManager::sharedState()->registerAccount(name, pass, email);
}
