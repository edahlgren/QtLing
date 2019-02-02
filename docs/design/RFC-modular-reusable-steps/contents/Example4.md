## Example 4

### Reading and Writing to JSON files.

This proposal recommends using the open-source, MIT-licensed [nlohmann::json](https://github.com/nlohmann/json) C++ library for reading and writing to JSON objects and files. Installation is very simple because the library is in a single file (header-only):

```
$ git clone git@github.com:nlohmann/json.git
$ sudo cp -r json/single_include/nlohmann /usr/local/include
```

To read a JSON object from a file, the previous examples use this pattern:

```c++
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::ifstream input("input.json");

json j;
input >> j;
```

To write a JSON object to a file, the previous examples use this pattern:

```c++
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

json j;
// ... populate the JSON object (see other examples).

std::ofstream output("output.json");
output << std::setw(4) << j << std::endl;

// note: setw(4) pretty-prints the JSON so it is more human readable.
```

#### The example

> **NOTE:** This example reads a JSON-formatted file into memory and pretty prints it to a different file so that the contents are easier to read.

[[C++ Code]](../examples/json_read_write.cpp)

#### Try the example

Build the example:

```
$ g++ -std=c++11 json_read_write.cpp -o test/json_read_write
```

Execute the example:

> **NOTE:** While not necessary, we recommend putting all generated files in the provided `test` directory. The provided `simple.json` file is a small JSON formatted file that is not pretty-printed.

```
$ cd test
$ ./json_read_write simple.json pretty.json
```

Inspect the `pretty.json` file to see the transformation:

```
$ cat pretty.json
{
    "answer": {
        "everything": 42
    },
    "happy": true,
    "list": [
        1,
        2,
        3
    ]
}
```

To learn more about the [`nlohmann::json`](https://github.com/nlohmann/json) library see:

+ [Documented examples](https://github.com/nlohmann/json#examples)
+ [Fully-executable examples](https://github.com/nlohmann/json/tree/develop/doc/examples)
+ [Doxygen documentation](https://nlohmann.github.io/json/)

[[Back to Table of Contents]](../README.md#table-of-contents)
