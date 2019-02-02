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

// A simple container for a word range in a sorted list
// of words.
struct word_range {
  size_t start;
  size_t end;
};

// Step operations (read input data, transform, write output data).
int read_input1(const char *input_filename, std::vector<std::string> &words);
int read_input2(const char *input_filename, std::vector<std::string> &protostems);
int transform_data(std::vector<std::string>& words,
                   std::vector<std::string>& protostems,
                   std::map<std::string, struct word_range>& protostem_ranges);
int write_output(const char *output_filename,
                 const char *wordlist_filename,
                 std::map<std::string, struct word_range>& ranges);

// g++ -std=c++11 suffix_sort_words.cpp -o suffix_sort_words
int main(int argc, char **argv) {
  // 0. Setup input and output files.
  if (argc < 2) {
    fprintf(stderr, "Usage: %s [words.json] [protostems.json]\n", argv[0]);    
    return 1;
  }
  std::string input1_filename(argv[1]);
  std::string input2_filename(argv[2]);
  const char *output_filename = "suffix_protostem_words.json";

  // 1. Try to parse the sorted word list from the first input file.
  std::vector<std::string> words;
  int error = read_input1(input1_filename.c_str(), words);
  if (error)
    return 1;

  // 1. Try to parse the protostems from the second input file.
  std::vector<std::string> protostems;
  error = read_input2(input2_filename.c_str(), protostems);
  if (error)
    return 1;
  
  // 2. Find the words associated with protostems.
  std::map<std::string, struct word_range> protostem_ranges;
  error = transform_data(words, protostems, protostem_ranges);
  if (error)
    return 1;

  // 3. Try to write the mappings of protostems to words to an output file.
  error = write_output(output_filename, input1_filename.c_str(),
                       protostem_ranges);
  if (error)
    return 1;

  // 4. Write out the filename for the next step to use.
  printf("%s\n", output_filename);
  
  return 0;
}
  
int read_input1(const char *input_filename, std::vector<std::string>& words) {
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

int read_input2(const char *input_filename, std::vector<std::string>& protostems) {
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
  if (tn != "Suffix.Protostems") {
    fprintf(stderr, "error: JSON object's typename must be \"Suffix.Protostems\", has \"%s\"\n", tn.c_str());
    return 1;
  }

  // Validate that the JSON is well-formed: typename of Suffix.Protostems
  // should always have a "protostems" element.
  if (!j.count("protostems")) {
    fprintf(stderr, "error: Suffix.Protostems must must contain key \"words\"\n");
    return 1;
  }

  // Parse the stems out of the protostems set.
  json jstems = j["protostems"];
  for (json::iterator it = jstems.begin(); it != jstems.end(); it++) {
    // Add the word.
    protostems.push_back(*it);
  }

  return 0;
}

struct word_range find_words_with_stem(std::vector<std::string>& words,
                                       std::string& stem, size_t start) {
  size_t stem_len = stem.size();

  while (start < words.size()) {
    std::string word = words.at(start);
    std::string potential_stem = word.substr(0, stem_len);
    if (!stem.compare(potential_stem)) { // Equal
      break;
    }
    start++;
  }

  size_t end = start;
  while (end < words.size()) {
    std::string word = words.at(start);
    std::string potential_stem = word.substr(0, stem_len);
    if (stem.compare(potential_stem)) { // Not Equal
      break;
    }
    end++;
  }

  struct word_range rng;
  rng.start = start;
  rng.end = end;
  return rng;
}

int transform_data(std::vector<std::string>& words,
                   std::vector<std::string>& protostems,
                   std::map<std::string, struct word_range>& protostem_ranges) {

  // Sort the protostems alphabetically.
  std::sort(protostems.begin(), protostems.end());

  size_t start = 0;
  for (auto stem : protostems) {
    // Find the range.
    struct word_range rng = find_words_with_stem(words, stem, start);

    // Add it to the output mappings.
    protostem_ranges.emplace(stem, rng);

    // Update the start index.
    start = rng.start;
  }

  return 0;
}

int write_output(const char *output_filename,
                 const char *wordlist_filename,
                 std::map<std::string, struct word_range>& ranges) {

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
  j["typename"] = "Suffix.ProtostemRanges";

  // Create a JSON object for the protostem range mappings.
  json jranges;
  std::map<std::string, struct word_range>::iterator it;
  for (it = ranges.begin(); it != ranges.end(); it++) {
    json jrange;
    jrange["word_start_index"] = it->second.start;
    jrange["word_end_index"] = it->second.end;

    //std::string stem = it->first;
    jranges.emplace(it->first, jrange);
  }

  j["protostem_to_words"] = jranges;
  j["wordlist"] = wordlist_filename;
  
  // Try to write the JSON object to the file.
  try {
    output << std::setw(4) << j << std::endl;
  } catch (std::exception& e) {
    fprintf(stderr, "error: failed to write json: %s\n", e.what());
    return 1;    
  }

  return 0;
}
