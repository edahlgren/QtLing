#include <exception>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

// Simple third-party header-only JSON library. To use:
//
// git clone git@github.com:nlohmann/json.git
// sudo cp -r nlohmann /usr/local/include
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Simple string utilities using the standard library.
#include "strutil.h"

// Neutral container for loading and sorting the word name
// and frequency of a line in a dx1 formatted file.
struct word {
  std::string name;
  int count;

  // For sorting.
  bool operator < (const word& w) const {
    return name < w.name;
  }
};

// Step operations (read input data, transform, write output data).
int read_input(const char *input_filename, std::vector<struct word> &words);
int transform_data(std::vector<struct word> &words);
int write_output(const char *output_filename, std::vector<struct word> &words);
  
// g++ -std=c++11 suffix_sort_words.cpp -o suffix_sort_words
int main(int argc, char **argv) {
  // 0. Setup input and output files.
  std::string input_filename = parse_strarg(argc, argv);
  if (input_filename.size() == 0) {
    fprintf(stderr, "Usage: %s [input.dx1]\n", argv[0]);    
  }
  const char *output_filename = "suffix_sorted_words.json";
  
  // 1. Try to parse the words and counts from the dx1 file.
  std::vector<struct word> words;
  int error = read_input(input_filename.c_str(), words);
  if (error)
    return 1;

  // 2. Try to transform (sort) the words.
  error = transform_data(words);
  if (error)
    return 1;

  // 3. Try to write the word list to an output file.
  error = write_output(output_filename, words);
  if (error)
    return 1;

  // 4. Write out the filename for the next step to use.
  printf("%s\n", output_filename);
  
  return 0;
}

int read_input(const char *input_filename, std::vector<struct word>& words) {
  std::ifstream input(input_filename);
  if (!input) {
    fprintf(stderr, "error: cannot open input file %s\n", input_filename);
    return 1;
  }

  // Keep track of line number for error reporting.
  int lineno = 0;
  
  // Try to read the dx1 file line by line.
  std::string line;
  while (std::getline(input, line)) {
    // Remove extra spaces.
    line = simplify(line);

    // Parse out the word and count parts of the line.
    std::vector<std::string> parts = split_space(line);

    // Check if the line is well-formed. The word part is required.
    if (parts.size() == 0) {
      fprintf(stderr, "error: line %d missing word part\n", lineno);
      return 1;
    }

    // Allocate a new word.
    struct word w;
    
    // Lowercase the name part.
    w.name = parts[0];
    lowercase(w.name);

    // Try to parse the count part.
    w.count = 1;
    if (parts.size() > 1) {
      w.count = std::stoi(parts[1], nullptr);
    }

    // Add it to the word list.
    words.push_back(w);

    // Increment the line number, for debug output on errors.
    lineno++;
  }

  return 0;
}

int transform_data(std::vector<struct word> &words) {
  // Sort the word list.
  std::sort(words.begin(), words.end());
  return 0;
}

int write_output(const char *output_filename, std::vector<struct word> &words) {
  // Try to open the output file, throwing away any previous content.
  std::ofstream output(output_filename, std::ios::trunc);  
  if (!output) {
    fprintf(stderr, "error: cannot open output file %s\n", output_filename);
    return 1;
  }
  
  // Create the JSON object.
  json j;
  
  // Tag it with a typename so that other processing steps can check
  // for correctly typed input.
  j["typename"] = "Suffix.SortedWords";

  // Create a JSON object for the word list.
  json jwordlist;
  for (auto w : words) {
    json jword;
    jword["name"] = w.name;
    jword["count"] = w.count;
    jwordlist.push_back(jword);
  }

  // And add the wordlist.
  j["words"] = jwordlist;
  
  // Try to write the JSON object to the file.
  try {
    output << std::setw(4) << j << std::endl;
  } catch (std::exception& e) {
    fprintf(stderr, "error: failed to write json: %s\n", e.what());
    return 1;    
  }

  return 0;
}
