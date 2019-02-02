#include <exception>
#include <fstream>
#include <iomanip>

// git clone git@github.com:nlohmann/json.git
// sudo cp -r nlohmann /usr/local/include
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// g++ -std=c++11 json_read_write.cpp -o json_read_write
int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s [input.json] [output.json]\n", argv[0]);
    return 1;
  }

  // Try to open the input file.
  char *input_filename = argv[1];
  std::ifstream input(input_filename);
  if (!input) {
    fprintf(stderr, "error: cannot open input file %s\n", input_filename);
    return 1;
  }
    
  // Try to read the JSON into memory.
  json j;
  try {
    input >> j;
  } catch (std::exception& e) {
    fprintf(stderr, "error: failed to parse json from file %s: %s\n", input_filename, e.what());
    return 1;
  }
  
  // Try to open or create the output file.
  char *output_filename = argv[2];
  std::ofstream output(output_filename);
  if (!output) {
    fprintf(stderr, "error: cannot open output file %s\n", output_filename);
    return 1;
  }
  
  // Try to write the JSON out to a file.
  try {
    output << std::setw(4) << j << std::endl;
  } catch (std::exception& e) {
    fprintf(stderr, "error: failed to write json to file %s: %s\n", output_filename, e.what());
    return 1;    
  }
  
  return 0;
}
