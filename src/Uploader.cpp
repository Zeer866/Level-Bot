#include "Uploader.hpp"

using namespace geode::prelude;

bool lb::uploadGeneratedLevel(int index, const std::string& levelString, int objectCount) {
    auto gm = GameLevelManager::sharedState();
    auto am = GJAccountManager::sharedState();
    if (am->m_accountID <= 0) return false; // hay que estar logueado en el GDPS

    auto prefix = Mod::get()->getSettingValue<std::string>("name-prefix");
    auto desc = Mod::get()->getSettingValue<std::string>("level-desc");

    auto level = GJGameLevel::create();
    level->m_levelName = fmt::format("{} #{}", prefix, index);
    level->m_levelDesc = ZipUtils::base64URLEncode(desc);
    level->m_levelString = levelString;
    level->m_levelType = GJLevelType::Editor;
    level->m_levelVersion = 1;
    level->m_levelLength = 2;
    level->m_audioTrack = 0;
    level->m_songID = 0;
    level->m_objectCount = objectCount;
    level->m_unlisted = false;

    gm->uploadLevel(level);
    return true;
}
