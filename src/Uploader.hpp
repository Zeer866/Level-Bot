#pragma once
#include <Geode/Geode.hpp>

namespace lb {
    // Crea y sube un nivel generado. Devuelve false si no hay sesión iniciada.
    bool uploadGeneratedLevel(int index, unsigned int seed, int objectCount);
}
