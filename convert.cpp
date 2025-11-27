#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <emscripten/emscripten.h>

void trim(std::string &s) {
  auto start = s.find_first_not_of(" \t\r\n");
  auto end = s.find_last_not_of(" \t\r\n");
  s = (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

bool is_number(const std::string &s) {
  if (s.empty()) return false;
  size_t start = (s[0] == '-' || s[0] == '+') ? 1 : 0;
  if (start >= s.size()) return false;
  bool has_dot = false;
  for (size_t i = start; i < s.size(); ++i) {
    if (s[i] == '.') {
      if (has_dot) return false;
      has_dot = true;
    } else if (!isdigit(s[i])) {
      return false;
    }
  }
  return true;
}

std::string round_number(const std::string &s, int decimals) {
  if (!is_number(s)) return s;
  if (decimals < 0) decimals = 0;
  
  double value = std::stod(s);
  double multiplier = std::pow(10.0, decimals);
  double rounded = std::round(value * multiplier) / multiplier;
  
  std::ostringstream oss;
  oss.precision(decimals);
  oss << std::fixed << rounded;
  return oss.str();
}

std::string round_significant_figures(const std::string &s, int sig_figs) {
  if (!is_number(s)) return s;
  if (sig_figs <= 0) sig_figs = 1;
  
  double value = std::stod(s);
  if (value == 0.0) return "0";
  
  double abs_value = std::abs(value);
  int exponent = std::floor(std::log10(abs_value));
  double multiplier = std::pow(10.0, sig_figs - 1 - exponent);
  double rounded = std::round(value * multiplier) / multiplier;
  
  // 整数として表示する必要があるか判定
  int decimal_places = std::max(0, sig_figs - 1 - exponent);
  
  std::ostringstream oss;
  if (decimal_places > 0) {
    oss.precision(decimal_places);
    oss << std::fixed << rounded;
  } else {
    oss << std::fixed << rounded;
    std::string result = oss.str();
    // 小数点以下の0を削除
    size_t dot_pos = result.find('.');
    if (dot_pos != std::string::npos) {
      result = result.substr(0, dot_pos);
    }
    return result;
  }
  return oss.str();
}

std::vector<std::string> split(const std::string &line) {
  std::vector<std::string> out;
  std::string cur;
  char delim = line.find('\t') != std::string::npos ? '\t' : ',';
  for (char c : line) {
    if (c == delim) { out.push_back(cur); cur.clear(); }
    else cur += c;
  }
  out.push_back(cur);
  return out;
}

typedef std::vector<std::vector<std::string>> Table;

Table parse(const std::string &input) {
  Table t;
  std::stringstream ss(input);
  std::string line;
  while (std::getline(ss, line)) {
    trim(line);
    if (line.empty()) continue;
    auto cells = split(line);
    for (auto &c : cells) trim(c);
    t.push_back(cells);
  }
  return t;
}

std::string escape(const std::string &s) {
  std::string out;
  for (char c : s) {
    if (c == '&' || c == '%' || c == '$' || c == '#' || c == '_' || c == '{' || c == '}') out += '\\';
    out += c;
  }
  return out;
}

std::string to_latex(const Table &t) {
  if (t.empty()) return "";
  std::string out = "\\begin{tabular}{" + std::string(t[0].size(), 'c') + "}\n\\hline\n";
  for (const auto &row : t) {
    for (size_t i = 0; i < row.size(); ++i) {
      if (i) out += " & ";
      out += escape(row[i]);
    }
    out += " \\\\\n";
  }
  return out + "\\hline\n\\end{tabular}";
}

std::string to_latex_rounded(const Table &t, int decimals) {
  if (t.empty()) return "";
  std::string out = "\\begin{tabular}{" + std::string(t[0].size(), 'c') + "}\n\\hline\n";
  for (const auto &row : t) {
    for (size_t i = 0; i < row.size(); ++i) {
      if (i) out += " & ";
      std::string cell = round_number(row[i], decimals);
      out += escape(cell);
    }
    out += " \\\\\n";
  }
  return out + "\\hline\n\\end{tabular}";
}

std::string to_latex_sig_figs(const Table &t, int sig_figs) {
  if (t.empty()) return "";
  std::string out = "\\begin{tabular}{" + std::string(t[0].size(), 'c') + "}\n\\hline\n";
  for (const auto &row : t) {
    for (size_t i = 0; i < row.size(); ++i) {
      if (i) out += " & ";
      std::string cell = round_significant_figures(row[i], sig_figs);
      out += escape(cell);
    }
    out += " \\\\\n";
  }
  return out + "\\hline\n\\end{tabular}";
}

std::string to_csv(const Table &t) {
  std::string out;
  for (size_t i = 0; i < t.size(); ++i) {
    for (size_t j = 0; j < t[i].size(); ++j) {
      if (j) out += ',';
      out += t[i][j];
    }
    if (i + 1 < t.size()) out += '\n';
  }
  return out;
}

std::string to_csv_rounded(const Table &t, int decimals) {
  std::string out;
  for (size_t i = 0; i < t.size(); ++i) {
    for (size_t j = 0; j < t[i].size(); ++j) {
      if (j) out += ',';
      std::string cell = round_number(t[i][j], decimals);
      out += cell;
    }
    if (i + 1 < t.size()) out += '\n';
  }
  return out;
}

std::string to_csv_sig_figs(const Table &t, int sig_figs) {
  std::string out;
  for (size_t i = 0; i < t.size(); ++i) {
    for (size_t j = 0; j < t[i].size(); ++j) {
      if (j) out += ',';
      std::string cell = round_significant_figures(t[i][j], sig_figs);
      out += cell;
    }
    if (i + 1 < t.size()) out += '\n';
  }
  return out;
}

char* dup(const std::string &s) {
  char *p = (char*)malloc(s.size() + 1);
  memcpy(p, s.c_str(), s.size() + 1);
  return p;
}

extern "C" {
EMSCRIPTEN_KEEPALIVE char* gen_latex(const char* in) {
  return in ? dup(to_latex(parse(in))) : dup("");
}
EMSCRIPTEN_KEEPALIVE char* gen_csv(const char* in) {
  return in ? dup(to_csv(parse(in))) : dup("");
}
EMSCRIPTEN_KEEPALIVE char* gen_latex_rounded(const char* in, int decimals) {
  return in ? dup(to_latex_rounded(parse(in), decimals)) : dup("");
}
EMSCRIPTEN_KEEPALIVE char* gen_csv_rounded(const char* in, int decimals) {
  return in ? dup(to_csv_rounded(parse(in), decimals)) : dup("");
}
EMSCRIPTEN_KEEPALIVE char* gen_latex_sig_figs(const char* in, int sig_figs) {
  return in ? dup(to_latex_sig_figs(parse(in), sig_figs)) : dup("");
}
EMSCRIPTEN_KEEPALIVE char* gen_csv_sig_figs(const char* in, int sig_figs) {
  return in ? dup(to_csv_sig_figs(parse(in), sig_figs)) : dup("");
}
}
