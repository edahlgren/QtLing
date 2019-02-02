#include <algorithm>
#include <iostream>
#include <regex>
#include <string>

// Remove leading, trailing, and extra spaces from std::string.
std::string simplify(std::string& s) {
  return std::regex_replace(s, std::regex("^ +| +$|( ) +"), "$1");
}

std::string& rtrim(std::string& str, const std::string& chars) {
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}

// Strip trailing newline from string.
std::string strip_newline(std::string& s) {
  return std::regex_replace(s, std::regex("\n"), "$1");
}

// Split string on " " delimiter.
std::vector<std::string> split_space(std::string& s) {
  std::string delim(" ");
 
  size_t start = 0U;
  size_t end = s.find(delim);

  std::vector<std::string> out;
  while (end != std::string::npos) {
    out.push_back(s.substr(start, end - start));
    start = end + delim.length();
    end = s.find(delim, start);
  }
  out.push_back(s.substr(start, end - start));
  return out;
}

// Lowercase the characters in a string.
void lowercase(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

std::string parse_strarg(int argc, char **argv) {
  if (argc < 2) {
    // Try to read the input file name from stdin.
    std::string input_line;
    if (!getline(std::cin, input_line)) {
      return std::string();
    }
    input_line = rtrim(input_line, "\n");
    return input_line;
  }

  return std::string(argv[1]);
}
