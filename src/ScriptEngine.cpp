#include "ScriptEngine.hpp"
#include "BotRunner.hpp"
#include "FloatingButton.hpp"
#include <Geode/Geode.hpp>
#include <algorithm>
#include <random>
#include <set>
#include <sstream>

using namespace geode::prelude;

static std::string trim(std::string s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

static const std::set<std::string> KNOWN = {
    "decir", "esperar", "poner", "sumar", "restar", "azar",
    "repetir", "si", "sino", "fin", "funcion", "llamar", "mientras",
    "stat", "enviar", "sonido", "nivel", "cuenta",
    "multiplicar", "dividir", "guardar", "cargar", "aleatorio", "notificar",
    "boton_color", "boton_escala", "boton_mover", "boton_reset"
};

static bool opensBlock(const std::string& cmd) {
    return cmd == "repetir" || cmd == "si" || cmd == "funcion" || cmd == "mientras";
}

int zs::Engine::value(const std::string& tok) {
    if (!tok.empty() && tok[0] == '$') {
        auto it = m_vars.find(tok.substr(1));
        return it == m_vars.end() ? 0 : it->second;
    }
    return geode::utils::numFromString<int>(tok).unwrapOr(0);
}

float zs::Engine::number(const std::string& tok) {
    if (!tok.empty() && tok[0] == '$') return static_cast<float>(value(tok));
    return geode::utils::numFromString<float>(tok).unwrapOr(0.f);
}

bool zs::Engine::compare(int a, const std::string& op, int b) {
    if (op == ">") return a > b;
    if (op == "<") return a < b;
    if (op == ">=") return a >= b;
    if (op == "<=") return a <= b;
    if (op == "==") return a == b;
    if (op == "!=") return a != b;
    return false;
}

bool zs::Engine::load(const std::string& src, std::string& error) {
    m_prog.clear();
    m_vars.clear();
    m_functions.clear();
    m_loops.clear();
    m_calls.clear();
    m_pc = 0;
    m_wait = 0.f;
    m_waitingBot = false;

    std::string norm = src;
    std::replace(norm.begin(), norm.end(), '\n', ';');

    std::stringstream ss(norm);
    std::string part;
    int line = 0;
    int depth = 0;

    while (std::getline(ss, part, ';')) {
        line++;
        part = trim(part);
        if (part.empty() || part[0] == '#') continue;

        Instr in;
        in.line = line;
        std::stringstream ts(part);
        ts >> in.cmd;
        std::transform(in.cmd.begin(), in.cmd.end(), in.cmd.begin(),
            [](unsigned char c) { return std::tolower(c); });

        if (!KNOWN.count(in.cmd)) {
            error = fmt::format("Comando {}: '{}' no existe", line, in.cmd);
            return false;
        }

        if (in.cmd == "decir" || in.cmd == "notificar") {
            std::string rest;
            std::getline(ts, rest);
            in.args.push_back(trim(rest));
        } else {
            std::string tok;
            while (ts >> tok) in.args.push_back(tok);
        }

        if (in.cmd == "funcion") {
            if (in.args.empty()) {
                error = fmt::format("Comando {}: 'funcion' necesita un nombre", line);
                return false;
            }
            m_functions[in.args[0]] = m_prog.size() + 1;
        }

        if (opensBlock(in.cmd)) depth++;
        if (in.cmd == "fin" && --depth < 0) {
            error = fmt::format("Comando {}: 'fin' sin bloque que cerrar", line);
            return false;
        }
        m_prog.push_back(in);
    }

    if (depth != 0) {
        error = "Falta un 'fin' al final";
        return false;
    }
    return true;
}

size_t zs::Engine::findBlockEnd(size_t start) {
    int depth = 0;
    size_t i = start;
    for (; i < m_prog.size(); i++) {
        if (opensBlock(m_prog[i].cmd)) depth++;
        if (m_prog[i].cmd == "fin" && --depth == 0) break;
    }
    return i;
}

bool zs::Engine::step(float dt, const std::function<void(const std::string&)>& say, std::string& error) {
    if (m_wait > 0.f) {
        m_wait -= dt;
        return true;
    }

    if (m_waitingBot) {
        lb::tickLevels(dt);
        lb::tickAccounts(dt);
        if (lb::levelsRunning() || lb::accountsRunning()) return true;
        m_waitingBot = false;
        m_pc++;
    }

    static const std::map<std::string, const char*> STATS = {
        {"estrellas", "6"}, {"diamantes", "13"}, {"lunas", "28"},
        {"monedas", "12"}, {"secretas", "8"}, {"demonios", "5"},
    };

    int budget = 500;
    while (m_pc < m_prog.size() && budget-- > 0) {
        auto& in = m_prog[m_pc];

        auto need = [&](size_t n) {
            if (in.args.size() < n) {
                error = fmt::format("Comando {}: a '{}' le faltan datos", in.line, in.cmd);
                return false;
            }
            return true;
        };

        if (in.cmd == "decir") {
            std::stringstream ws(in.args[0]);
            std::string word, out;
            while (ws >> word) {
                if (word[0] == '$') word = std::to_string(value(word));
                out += (out.empty() ? "" : " ") + word;
            }
            say(out);
            m_pc++;
        } else if (in.cmd == "notificar") {
            std::stringstream ws(in.args[0]);
            std::string word, out;
            while (ws >> word) {
                if (word[0] == '$') word = std::to_string(value(word));
                out += (out.empty() ? "" : " ") + word;
            }
            FLAlertLayer::create("Script", out.c_str(), "OK")->show();
            m_pc++;
        } else if (in.cmd == "esperar") {
            if (!need(1)) return false;
            m_wait = number(in.args[0]);
            m_pc++;
            return true;
        } else if (in.cmd == "poner") {
            if (!need(2)) return false;
            m_vars[in.args[0]] = value(in.args[1]);
            m_pc++;
        } else if (in.cmd == "sumar") {
            if (!need(2)) return false;
            m_vars[in.args[0]] += value(in.args[1]);
            m_pc++;
        } else if (in.cmd == "restar") {
            if (!need(2)) return false;
            m_vars[in.args[0]] -= value(in.args[1]);
            m_pc++;
        } else if (in.cmd == "multiplicar") {
            if (!need(2)) return false;
            m_vars[in.args[0]] *= value(in.args[1]);
            m_pc++;
        } else if (in.cmd == "dividir") {
            if (!need(2)) return false;
            int d = value(in.args[1]);
            if (d != 0) m_vars[in.args[0]] /= d;
            m_pc++;
        } else if (in.cmd == "guardar") {
            if (!need(1)) return false;
            Mod::get()->setSavedValue<int>("script:" + in.args[0], value("$" + in.args[0]));
            m_pc++;
        } else if (in.cmd == "cargar") {
            if (!need(1)) return false;
            m_vars[in.args[0]] = Mod::get()->getSavedValue<int>("script:" + in.args[0], 0);
            m_pc++;
        } else if (in.cmd == "azar") {
            if (!need(3)) return false;
            int lo = value(in.args[1]);
            int hi = value(in.args[2]);
            if (hi < lo) std::swap(lo, hi);
            static std::mt19937 rng{std::random_device{}()};
            m_vars[in.args[0]] = std::uniform_int_distribution<int>(lo, hi)(rng);
            m_pc++;
        } else if (in.cmd == "aleatorio") {
            if (in.args.size() < 2) {
                error = fmt::format("Comando {}: 'aleatorio' necesita variable y al menos un valor", in.line);
                return false;
            }
            static std::mt19937 rng2{std::random_device{}()};
            std::uniform_int_distribution<size_t> pick(1, in.args.size() - 1);
            m_vars[in.args[0]] = value(in.args[pick(rng2)]);
            m_pc++;
        } else if (in.cmd == "repetir") {
            if (!need(1)) return false;
            int n = value(in.args[0]);
            if (n <= 0) {
                m_pc = findBlockEnd(m_pc) + 1;
            } else {
                m_loops.push_back({m_pc + 1, n, false});
                m_pc++;
            }
        } else if (in.cmd == "mientras") {
            if (!need(3)) return false;
            bool keepGoing = compare(value(in.args[0]), in.args[1], value(in.args[2]));
            if (keepGoing) {
                m_loops.push_back({m_pc, 0, true});
                m_pc++;
            } else {
                m_pc = findBlockEnd(m_pc) + 1;
            }
        } else if (in.cmd == "si") {
            if (!need(3)) return false;
            bool ok = compare(value(in.args[0]), in.args[1], value(in.args[2]));
            if (ok) {
                m_pc++;
            } else {
                int depth = 0;
                size_t i = m_pc + 1;
                size_t jump = 0;
                for (; i < m_prog.size(); i++) {
                    if (opensBlock(m_prog[i].cmd)) depth++;
                    else if (m_prog[i].cmd == "fin") {
                        if (depth == 0) { jump = i + 1; break; }
                        depth--;
                    } else if (m_prog[i].cmd == "sino" && depth == 0) {
                        jump = i + 1;
                        break;
                    }
                }
                m_pc = jump ? jump : m_prog.size();
            }
        } else if (in.cmd == "sino") {
            m_pc = findBlockEnd(m_pc) + 1;
        } else if (in.cmd == "fin") {
            if (!m_loops.empty() && m_loops.back().start <= m_pc) {
                auto loop = m_loops.back();
                m_loops.pop_back();
                if (loop.isWhile) {
                    m_pc = loop.start; // vuelve al 'mientras' para revisar de nuevo
                } else if (--loop.left > 0) {
                    m_loops.push_back(loop);
                    m_pc = loop.start;
                } else {
                    m_pc++;
                }
            } else if (!m_calls.empty()) {
                m_pc = m_calls.back().returnPc;
                m_calls.pop_back();
            } else {
                m_pc++;
            }
        } else if (in.cmd == "funcion") {
            m_pc = findBlockEnd(m_pc) + 1;
        } else if (in.cmd == "llamar") {
            if (!need(1)) return false;
            auto it = m_functions.find(in.args[0]);
            if (it == m_functions.end()) {
                error = fmt::format("Comando {}: funcion '{}' no existe", in.line, in.args[0]);
                return false;
            }
            m_calls.push_back({m_pc + 1});
            m_pc = it->second;
        } else if (in.cmd == "stat") {
            if (!need(2)) return false;
            auto it = STATS.find(in.args[0]);
            if (it == STATS.end()) {
                error = fmt::format("Comando {}: stat '{}' no existe", in.line, in.args[0]);
                return false;
            }
            GameStatsManager::sharedState()->setStat(it->second, value(in.args[1]));
            m_pc++;
        } else if (in.cmd == "enviar") {
            GameLevelManager::sharedState()->updateUserScore();
            m_pc++;
        } else if (in.cmd == "sonido") {
            FMODAudioEngine::sharedEngine()->playEffect("achievement_01.ogg");
            m_pc++;
        } else if (in.cmd == "nivel") {
            if (!need(1)) return false;
            lb::runLevels(value(in.args[0]), nullptr);
            m_waitingBot = true;
            return true;
        } else if (in.cmd == "cuenta") {
            if (!need(1)) return false;
            lb::runAccounts(value(in.args[0]), nullptr);
            m_waitingBot = true;
            return true;
        } else if (in.cmd == "boton_color") {
            if (!need(3)) return false;
            FloatingButton::setTint(value(in.args[0]), value(in.args[1]), value(in.args[2]));
            m_pc++;
        } else if (in.cmd == "boton_escala") {
            if (!need(1)) return false;
            FloatingButton::setButtonScale(number(in.args[0]));
            m_pc++;
        } else if (in.cmd == "boton_mover") {
            if (!need(2)) return false;
            FloatingButton::setButtonPos(number(in.args[0]), number(in.args[1]));
            m_pc++;
        } else if (in.cmd == "boton_reset") {
            FloatingButton::resetButton();
            m_pc++;
        }
    }
    return m_pc < m_prog.size();
}
