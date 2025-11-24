#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <emscripten/emscripten.h>

class StringUtils {
public:
    static void trim(std::string &s) {
        auto start = s.find_first_not_of(" \t\r\n");
        auto end = s.find_last_not_of(" \t\r\n");
        s = (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }

    static std::vector<std::string> split(const std::string &line, char delimiter) {
        std::vector<std::string> out;
        std::string cur;
        std::stringstream ss(line);
        std::string item;
        
        for (char c : line) {
            if (c == delimiter) { out.push_back(cur); cur.clear(); }
            else cur += c;
        }
        out.push_back(cur);
        return out;
    }

    static std::string escapeLatex(const std::string &s) {
        std::string out;
        out.reserve(s.size() * 1.2);
        for (char c : s) {
            if (c == '&' || c == '%' || c == '$' || c == '#' || c == '_' || c == '{' || c == '}') {
                out += '\\';
            }
            out += c;
        }
        return out;
    }
};

class Table {
public:
    using Row = std::vector<std::string>;
    using Grid = std::vector<Row>;

private:
    Grid m_data;

public:
    explicit Table(const std::string &input) {
        parse(input);
    }

    Table() = default;

    bool empty() const {
        return m_data.empty();
    }

    size_t rowCount() const {
        return m_data.size();
    }

    size_t colCount() const {
        return m_data.empty() ? 0 : m_data[0].size();
    }

    const Grid& getData() const {
        return m_data;
    }

    std::string toLatex() const {
        if (empty()) return "";
        
        size_t cols = colCount();
        std::string out = "\\begin{tabular}{" + std::string(cols, 'c') + "}\n\\hline\n";
        
        for (const auto &row : m_data) {
            for (size_t i = 0; i < row.size(); ++i) {
                if (i > 0) out += " & ";
                if (i < row.size()) {
                    out += StringUtils::escapeLatex(row[i]);
                }
            }
            out += " \\\\\n";
        }
        out += "\\hline\n\\end{tabular}";
        return out;
    }

    std::string toCSV() const {
        std::string out;
        for (size_t i = 0; i < m_data.size(); ++i) {
            for (size_t j = 0; j < m_data[i].size(); ++j) {
                if (j > 0) out += ',';
                out += m_data[i][j];
            }
            if (i + 1 < m_data.size()) out += '\n';
        }
        return out;
    }

private:
    void parse(const std::string &input) {
        m_data.clear();
        std::stringstream ss(input);
        std::string line;
        
        while (std::getline(ss, line)) {
            StringUtils::trim(line);
            if (line.empty()) continue;
            
            char punctuation = (line.find('\t') != std::string::npos) ? '\t' : ',';
            
            auto cells = StringUtils::split(line, punctuation);
            for (auto &c : cells) StringUtils::trim(c);
            
            m_data.push_back(cells);
        }
    }
};

char* allocateString(const std::string &s) {
    char *p = (char*)malloc(s.size() + 1);
    if (!p) return nullptr;
    std::memcpy(p, s.c_str(), s.size() + 1);
    return p;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE char* gen_latex(const char* in) {
    if (!in) return allocateString("");
    Table table(in);
    return allocateString(table.toLatex());
}

EMSCRIPTEN_KEEPALIVE char* gen_csv(const char* in) {
    if (!in) return allocateString("");
    Table table(in);
    return allocateString(table.toCSV());
}

}