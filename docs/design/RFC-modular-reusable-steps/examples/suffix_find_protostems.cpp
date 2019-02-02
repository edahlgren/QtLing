#include <exception>
#include <fstream>
#include <iomanip>
#include <set>
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

// Step operations (read input data, transform, write output data).
int read_input(const char *input_filename, std::vector<std::string> &words);
int transform_data(std::vector<std::string>& words, std::set<std::string>& protostems);
int write_output(const char *output_filename, std::set<std::string>& protostems);

// g++ -std=c++11 suffix_sort_words.cpp -o suffix_sort_words
int main(int argc, char **argv) {
  // 0. Setup input and output files.
  std::string input_filename = parse_strarg(argc, argv);
  if (input_filename.size() == 0) {
    fprintf(stderr, "Usage: %s [input.json]\n", argv[0]);    
  }
  const char *output_filename = "suffix_protostems.json";

  // 1. Try to parse the sorted word list from the input file.
  std::vector<std::string> words;
  int error = read_input(input_filename.c_str(), words);
  if (error)
    return 1;

  // 2. Try to transform the word list to a set of protostems.
  std::set<std::string> protostems;
  error = transform_data(words, protostems);
  if (error)
    return 1;

  // 3. Try to write the protostem set to an output file.
  error = write_output(output_filename, protostems);
  if (error)
    return 1;

  // 4. Write out the filename for the next step to use.
  printf("%s\n", output_filename);
  
  return 0;
}
  
int read_input(const char *input_filename, std::vector<std::string>& words) {
  // Try to open the file.
  std::ifstream input(input_filename);
  if (!input) {
    fprintf(stderr, "error: cannot open input file %s\n", input_filename);
    return 1;
  }

  // Parse the JSON.
  json j;
  try {
    input >> j;
  } catch (std::exception& e) {
    fprintf(stderr, "error: failed to parse json from file %s: %s\n", input_filename, e.what());
    return 1;
  }

  // Validate that we have the right type of JSON.
  if (!j.count("typename")) {
    fprintf(stderr, "error: JSON object must contain key \"typename\"\n");
    return 1;
  }
  std::string tn = j["typename"].get<std::string>();
  if (tn != "Suffix.SortedWords") {
    fprintf(stderr, "error: JSON object's typename must be \"Suffix.SortedWords\", has \"%s\"\n", tn.c_str());
    return 1;
  }

  // Validate that the JSON is well-formed: typename of Suffix.SortedWords
  // should always have a "words" element.
  if (!j.count("words")) {
    fprintf(stderr, "error: Suffix.SortedWords must must contain key \"words\"\n");
    return 1;
  }

  // Parse the words out of the sorted word list.
  json jwordlist = j["words"];
  for (json::iterator it = jwordlist.begin(); it != jwordlist.end(); it++) {
    json jword = *it;

    // Validate that the words are well-formed. A word should always have a
    // "name" element.
    if (!jword.count("name")) {
      fprintf(stderr, "error: words in Suffix.SortedWords must must contain key \"name\"\n");
      return 1;
    }
    
    // Add the word.
    words.push_back(jword["name"].get<std::string>());
  }

  return 0;
}

#define MINIMUM_STEM_LENGTH 4

std::string find_stem(std::string& previous, std::string& current) {
  // Try to find a protostem by comparing each of the
  // characters up to the end of the shortest word.
  size_t end = std::min(current.size(), previous.size());

  for (size_t charno = 0; charno < end; charno++) {
    // Skip stems that would be too short.
    if (charno < MINIMUM_STEM_LENGTH)
      continue;
    
    if (previous.at(charno) != current.at(charno)) {
      // Extract the left-most charno characters. Should always be
      // non-empty.
      return previous.substr(0, charno);
    }
  }

  // Return the previous word if it's long enough.
  if (previous.size() >= MINIMUM_STEM_LENGTH)
    return previous;

  // The empty string means that we didn't find a stem.
  return std::string();
}

int transform_data(std::vector<std::string>& words, std::set<std::string>& protostems) {
  std::string current, previous;
  for (size_t wordno = 0; wordno < words.size(); wordno++) {
    // Update the current word.
    current = words.at(wordno);

    // Skip the first word to make previous valid the second
    // time around.
    if (wordno > 0) {
      // Try to find a stem.
      std::string stem = find_stem(previous, current);
      if (stem.size() > 0) {
        // Insert the stem if it is not already present.
        if (!protostems.count(stem))
          protostems.insert(stem);
      }
    }

    // Update the previous word.
    previous = current;
  }

  return 0;
}

int write_output(const char *output_filename, std::set<std::string>& protostems) {
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
  j["typename"] = "Suffix.Protostems";
  // STL containers with standard types can auto-convert to JSON arrays.
  j["protostems"] = protostems;

  // Try to write the JSON object to the file.
  try {
    output << std::setw(4) << j << std::endl;
  } catch (std::exception& e) {
    fprintf(stderr, "error: failed to write json: %s\n", e.what());
    return 1;    
  }

  return 0;
}
