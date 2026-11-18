#include <Geode/Geode.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include <Geode/modify/GJAccountManager.hpp>
#include "State.hpp"

using namespace geode::prelude;

class $modify(LBGameLevelManager, GameLevelManager) {
    void handleIt(bool success, gd::string response, gd::string tag, GJHttpType type) {
        GameLevelManager::handleIt(success, response, tag, type);

        if (type == GJHttpType::UploadLevel) {
            std::string resp = response;
            int id = geode::utils::numFromString<int>(resp).unwrapOr(-1);
            lb::notifyUpload(success && id > 0);
        }
    }
};

class $modify(LBAccountManager, GJAccountManager) {
    void handleIt(bool success, gd::string response, gd::string tag, GJHttpType type) {
        GJAccountManager::handleIt(success, response, tag, type);

        if (type == GJHttpType::RegisterAccount) {
            lb::notifyAccount(success ? std::string(response) : std::string("-1"));
        }
    }
};
