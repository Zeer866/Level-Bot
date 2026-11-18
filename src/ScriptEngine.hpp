#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace zs {
    struct Instr {
        std::string cmd;
        std::vector<std::string> args;
        int line = 0;
    };

    class Engine {
    public:
        bool load(const std::string& src, std::string& error);
        // Devuelve true mientras el script siga corriendo.
        bool step(float dt, const std::function<void(const std::string&)>& say, std::string& error);

    private:
        struct Loop { size_t start; int left; };
        std::vector<Instr> m_prog;
        std::map<std::string, int> m_vars;
        std::vector<Loop> m_loops;
        size_t m_pc = 0;
        float m_wait = 0.f;

        int value(const std::string& tok);
        float number(const std::string& tok);
    };
}
