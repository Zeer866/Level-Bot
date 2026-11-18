#pragma once
#include <Geode/Geode.hpp>
#include <string>

namespace lb {
    // Sube un nivel con el string ya generado. Devuelve false si no hay sesión.
    bool uploadGeneratedLevel(int index, const std::string& levelString, int objectCount);
}
