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
        bool step(float dt, const std::function<void(const std::string&)>& say, std::string& error);

    private:
        struct Loop { size_t start; int left; bool isWhile; };
        struct Frame { size_t returnPc; };

        std::vector<Instr> m_prog;
        std::map<std::string, int> m_vars;
        std::map<std::string, size_t> m_functions;
        std::vector<Loop> m_loops;
        std::vector<Frame> m_calls;
        size_t m_pc = 0;
        float m_wait = 0.f;
        bool m_waitingBot = false;

        int value(const std::string& tok);
        float number(const std::string& tok);
        bool compare(int a, const std::string& op, int b);
        size_t findBlockEnd(size_t start);
    };
}
