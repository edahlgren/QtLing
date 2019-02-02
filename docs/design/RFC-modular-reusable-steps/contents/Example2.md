## Example 2

### CLexicon::step1_from_words_to_protostems (first half)

The first half of `CLexicon::step1_from_words_to_protostems` populates a set of protostems by comparing adjacent pairs of words in a sorted word list. From [lexicon_crab1.cpp](https://github.com/edahlgren/QtLing/blob/6df4bf4898274a26db7fc961f4cc7e8f7c0a91e/QtLing/lexicon_crab1.cpp#L76):

```c++
void CLexicon::step1_from_words_to_protostems() {
    // ... omitted for clarity

    int temp_j = 0;
    for (int wordno=0; wordno<Words->size(); wordno ++) {
        temp_j++;
        if (temp_j++ == 5000) {
            temp_j = 0;
            m_ProgressBar->setValue(wordno);
            qApp->processEvents();
        }

        this_word = Words->at(wordno);
        this_word_length = this_word.length();
        if (StartFlag){
            StartFlag = false;
            previous_word = this_word;
            previous_word_length = this_word_length;
            continue;
        }
        DifferenceFoundFlag = false;
        int end = qMin(this_word_length, previous_word_length);
        if (m_SuffixesFlag){   // -->   Suffix case   <-- //
            for (int i=0; i <end; i++){
                if (i < M_MINIMUM_STEM_LENGTH ) continue;
                if (previous_word[i] != this_word[i]){
                    potential_stem = previous_word.left(i);
                    if (potential_stem.length()== 0){continue;}
                    DifferenceFoundFlag = true;
                    if (!m_suffix_protostems.contains(potential_stem)){
                        m_suffix_protostems[potential_stem] = new protostem(potential_stem, true);
                    }
                    break;
                } // end of having found a difference.
            }// end of loop over this word;
            if (DifferenceFoundFlag == false){
                if (!m_suffix_protostems.contains(previous_word)
                        && previous_word_length >= M_MINIMUM_STEM_LENGTH){
                    m_suffix_protostems[previous_word] = new protostem(previous_word, true);
                }
            }
        }  // end of suffix case.
        // ... omitted for clarity
    } // word loop.
    // ... omitted for clarity (end of first half)
}
```

#### The example

[[C++ Code]](https://github.com/edahlgren/QtLing/blob/master/docs/design/RFC-modular-reusable-steps/examples/suffix_find_protostems.cpp)

> **NOTE:** This example only covers code that executes when finding suffixes, *not prefixes*. It may be helpful to keep the cases separate and put common code in a small library.

+ **Input file**: a JSON formated file containing sorted words and their frequencies

+ **Data transformation**: find the set of protostems (for a suffix search)

+ **Output file**: JSON formatted file containing the set of protostems

#### Try the example

Build the example:

```
$ g++ -std=c++11 suffix_find_protostems.cpp -o test/suffix_find_protostems
```

Execute the example:

> **NOTE:** Execute the previous example to generate a `suffix_sorted_words.json` file.

```
$ cd test
$ ./suffix_find_protostems suffix_sorted_words.json
suffix_protostems.json
```

Inspect the JSON data to see the protostems. If using the `suffix_sorted_words.json` file from the previous example, expect to see:

```
$ cat suffix_protostems.json
{
    "protostems": [
        "that"
    ],
    "typename": "Suffix.Protostems"
}
```

#### Understanding the example

Working backwards again, the JSON file has a `typename` field:

```
    "typename": "Suffix.Protostems"
```

This field that tells consumers that this JSON file contains protostems suitable for finding suffixes.

> **NOTE:** The name "Suffix.Protostems" is arbitrary. It can be anything that uniquely describes the data.

The JSON file also has a `protostems` field:

```
    "protostems": [
        "that"
    ],
```

This field contains the set of protostems as a JSON array of strings, where each string is a protostem.

> **NOTE:** This JSON data can have a different structure, as long as it can easily be parsed by C++ and Python code into a set-like data structure. For example this format would also work:
>
> ```
>    "protostems": {
>        "that": true
>    },
> ```
>
> Where the protostems are the keys of a JSON object and `true` indicates that the entry exists in the set. One could argue that this is alternative format is unnecessarily more verbose than the JSON used in this example.

Digging into the example code, it first checks that the input file contains a JSON object with a `typename` equal to "Suffix.SortedWords". This shows how to make use of the `typename` field:

```c++
int read_input(const char *input_filename, std::vector<std::string>& words) {
  // ... omitted for clarity.

  // Parse the JSON.
  json j;
  try {
    input >> j;
    // ... omitted for clarity.

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

  // ... omitted for clarity.
}
```

Then it parses words in the JSON object into a vector of strings:

```c++
int read_input(const char *input_filename, std::vector<std::string>& words) {
  // ... omitted for clarity.

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

  // ... omitted for clarity.
}
```

Next it does the "data transformation" step: finding a set of protostems from the sorted word list. This happens in two short functions:

```c++
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
```

```c++
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
```

Finally it constructs a JSON object out of the set of protostems and writes that object to a file:

```c++
int write_output(const char *output_filename, std::set<std::string>& protostems) {
  // ... omitted for clarity.

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
    // ... omitted for clarity.
}
```

To better understand the JSON operations, see [Reading and Writing to JSON files](./Example4.md).

[[Go to Example 3]](./Example3.md)

[Back to Table of Contents](../README.md)
