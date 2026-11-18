#pragma once
#include <functional>
#include <string>

namespace lb {
    // Sube "count" niveles. onDone(subidos, fallidos) se llama al terminar.
    void runLevels(int count, std::function<void(int, int)> onDone);
    // Avanza el proceso de niveles un paso; llamar cada frame mientras esté activo.
    void tickLevels(float dt);
    bool levelsRunning();

    // Crea "count" cuentas. onDone(creadas, fallidas) se llama al terminar.
    void runAccounts(int count, std::function<void(int, int)> onDone);
    void tickAccounts(float dt);
    bool accountsRunning();
}
