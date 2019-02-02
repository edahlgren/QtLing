## Example 3

### CLexicon::step1_from_words_to_protostems (second half)

The second half of `CLexicon::step1_from_words_to_protostems` maps protostems to words in a sorted word list. Instead of storing copies or pointers to the words themselves, the code stores a start and end index in the sorted word list, representing a range of words. From [lexicon_crab1.cpp](https://github.com/edahlgren/QtLing/blob/6df4bf4898274a26db7fc961f4cc7e8f7c0a91e/QtLing/lexicon_crab1.cpp#L163):

```c++
void CLexicon::step1_from_words_to_protostems() {
    // ... omitted for clarity

    if (m_SuffixesFlag){
        // ... omitted for clarity

        QStringList alphabetized_protostems = m_suffix_protostems.keys();
        alphabetized_protostems.sort();
        int alpha = 0;
        //int omega = 0;
        //int start_index = 0;

        for (int p = 0; p<alphabetized_protostems.length(); p++){
            QString this_protostem_t = alphabetized_protostems[p];
            int protostem_length = this_protostem_t.length();
            int i = alpha;
            while (Words->at(i).left(protostem_length) != this_protostem_t){
                i++;
            }
            int j = i;
            alpha = i;
            while (j < Words->length() && Words->at(j).left(protostem_length) == this_protostem_t){
                j++;
            }
            m_suffix_protostems[this_protostem_t]->set_start_and_end_word(i,j-1);
        }
    } // end of suffix case

    // ... omitted for clarity
}
```

#### The example

[[C++ Code]](https://github.com/edahlgren/QtLing/blob/master/docs/design/RFC-modular-reusable-steps/examples/suffix_find_protostem_words.cpp)

> **NOTE:** This example only covers code that executes when finding suffixes, *not prefixes*. It may be helpful to keep the cases separate and put common code in a small library.

+ **Input file 1**: a JSON formated file containing sorted words and their frequencies

+ **Input file 2**: a JSON formated file containing a set of protostems

+ **Data transformation**: associate protostems with words (for a suffix search)

+ **Output file**: JSON formatted file containing a mapping of protostems to word ranges in a sorted word list

#### Try the example

Build the example:

```
$ g++ -std=c++11 suffix_find_protostem_words.cpp -o test/suffix_find_protostem_words
```

Execute the example:

> **NOTE:** Execute the previous two examples to generate a `suffix_sorted_words.json` file and a `suffix_protostems.json` file.

```
$ cd test
$ ./suffix_find_protostem_words suffix_sorted_words.json suffix_protostems.json
suffix_protostem_words.json
```

Inspect the JSON data to see the mappings of protostems to words. If using the `suffix_sorted_words.json` file and the `suffix_protostems.json` file from the previous two examples, expect to see:

```
$ cat suffix_protostem_words.json
{
    "protostem_to_words": {
        "that": {
            "word_end_index": 10,
            "word_start_index": 6
        }
    },
    "typename": "Suffix.ProtostemRanges",
    "wordlist": "suffix_sorted_words.json"
}
```

#### Understanding the example

Working backwards again, the JSON file has a `typename` field:

```
    "typename": "Suffix.ProtostemRanges",
```

This field that tells consumers that this JSON file contains a mapping of protostems to word ranges in a sorted word list suitable for finding suffixes.

> **NOTE:** The name "Suffix.ProtostemRanges" is arbitrary. It can be anything that uniquely describes the data.

The JSON file has a `protostem_to_words` field that holds the mappings:

```
    "protostem_to_words": {
        "that": {
            "word_end_index": 10,
            "word_start_index": 6
        }
    },
```

> **NOTE:** This JSON data can have a different structure, as long as it can easily be parsed by C++ and Python code into a map-like structure. For example this format would also work:
>
> ```
>    "protostem_to_words": [
>        {
>            "protostem": "that",
>            "word_end_index": 10,
>            "word_start_index": 6
>        }
>    ],
> ```
>
> Where the protostem is labeled like the start and end word index in a single JSON object. One could argue that this alternative format makes it less obvious that the "protostem" field is intended to be the key in a map.

Finally, the JSON file has an extra field called `wordlist`:

```
    "wordlist": "suffix_sorted_words.json"
```

This is the name of the file containing the sorted word list associated with a word index. Without this field, it would be difficult to interpret the word ranges on their own.


Digging into the example code, it first reads the sorted word list and the set of protostems from JSON files, checking that the JSON objects contain a `typename` equal to "Suffix.SortedWords" and "Suffix.Protostems" respectively. It parses the sorted word list into a vector of strings:

```c++
int read_input1(const char *input_filename, std::vector<std::string>& words) {
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

And the protostem set into a vector of strings as well, a container that is easy to sort:

```c++
int read_input2(const char *input_filename, std::vector<std::string>& protostems) {
  // ... omitted for clarity.

  // Parse the stems out of the protostems set.
  json jstems = j["protostems"];
  for (json::iterator it = jstems.begin(); it != jstems.end(); it++) {
    // Add the word.
    protostems.push_back(*it);
  }

  // ... omitted for clarity.
}
```

Then it sorts the protostems and alphabetically searches for words containing each protostem. This happens in two short functions:

```c++
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
```

```c++
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
```

Finally it constructs a JSON object out of the mappings of protostems to word ranges and writes that object to a file, including the filename of the wordlist to be used to interpret the word ranges:

```c++
int write_output(const char *output_filename,
                 const char *wordlist_filename,
                 std::map<std::string, struct word_range>& ranges) {
  // ... omitted for clarity.

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
    // ... omitted for clarity.
}
```

To better understand the JSON operations, see [Reading and Writing to JSON files](./Example4.md).

[Go to Example 4](./Example4.md)

[Back to Table of Contents](../README.md#table-of-contents)
