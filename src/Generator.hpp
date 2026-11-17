#pragma once
#include <string>

namespace lb {
    // Devuelve el string de nivel ya comprimido (listo para m_levelString).
    std::string generateLevel(unsigned int seed, int objectCount);
}
