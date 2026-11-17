#include "Generator.hpp"
#include <Geode/Geode.hpp>
#include <random>
#include <sstream>

using namespace geode::prelude;

std::string lb::generateLevel(unsigned int seed, int objectCount) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> pick(0, 99);

    std::ostringstream ss;
    // Cabecera mínima (colores, modo de juego, etc.)
    ss << "kS38,1_40_2_125_3_255_11_255_12_255_13_255_4_-1_6_1000_7_1_15_1_18_0_8_1|";

    float x = 150.f;
    int placed = 0;
    while (placed < objectCount) {
        int r = pick(rng);
        if (r < 55) {
            // bloque en el suelo (ID 1)
            ss << "1,1,2," << x << ",3,15;";
            placed++;
        } else if (r < 85) {
            // pincho (ID 8)
            ss << "1,8,2," << x << ",3,15;";
            placed++;
        } else {
            // bloque + pincho encima
            ss << "1,1,2," << x << ",3,15;";
            ss << "1,8,2," << x << ",3,45;";
            placed += 2;
        }
        x += 30.f * (1 + (pick(rng) % 4)); // separación variable
    }
    return ZipUtils::compressString(ss.str(), false, 0);
}
