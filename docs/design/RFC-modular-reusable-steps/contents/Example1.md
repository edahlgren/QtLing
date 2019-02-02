## Example 1

### MainWindow::read_dx1_file

`MainWindow::read_dx1_file` reads a [DX1](https://github.com/markandrus/DX1/blob/master/README.md) formated file, parses the file line-by-line into a word list, and sorts the word list in two ways (see `CWordCollection::sort_word_list` for more detail). From [mainwindow.cpp](https://github.com/edahlgren/QtLing/blob/6df4bf4898274a26db7fc961f4cc7e8f7c0a91e/QtLing/mainwindow.cpp#L610):

```c++
void MainWindow::read_dx1_file()
{
     QFile file(m_name_of_data_file);

     // ... omitted for clarity

     QTextStream in(&file);
     CWordCollection * Words = get_lexicon()->get_word_collection();

     while (!in.atEnd())
     {
            QString line = in.readLine();
            line = line.simplified(); // get rid of extra spaces
            QStringList words = line.split(" ");
            QString word = words[0];
            word = word.toLower();
            CWord* pWord = *Words <<  word;
            //qDebug() << 486 << word<< 486;
            if (words.size()> 1) {
                pWord->SetWordCount(words[1].toInt());
                //if (words[1].toInt() > 1000)
                   // qDebug() << 489 << words[0] << words[1];
            }
     }
    Words->sort_word_list();

    // ... omitted for clarity
}
```

#### The example

> **NOTE:** This example only covers code that executes when finding suffixes, *not prefixes*. It may be helpful to keep the cases separate and put common code in a small library.

[[C++ Code]](../examples/suffix_sort_words.cpp)

+ **Input file**: a DX1 formated file containing words and optionally word frequencies

+ **Data transformation**: parse and sort words (for a suffix search)

+ **Output file**: JSON formatted file containing sorted words and their frequencies

#### Try the example

Build the example:

```
$ g++ -std=c++11 suffix_sort_words.cpp -o test/suffix_sort_words
```

Execute the example:

> **NOTE:** While not necessary, we recommend putting all generated files in the provided `test` directory. The provided `test.dx1` file is the first 10 lines of [browncorpus.dx1](https://github.com/edahlgren/QtLing/blob/master/QtLing/browncorpus.dx1).

```
$ cd test
$ ./suffix_sort_words test.dx1
suffix_sorted_words.json
```

Inspect the JSON data to see the sorted word list:

```
$ cat suffix_sorted_words.json
{
    "typename": "Suffix.SortedWords",
    "words": [
        {
            "count": 21970,
            "name": "a"
        },
        {
            "count": 27624,
            "name": "and"
        },
        {
            "count": 8817,
            "name": "for"
        },
        {
            "count": 19451,
            "name": "in"
        },
        {
            "count": 9775,
            "name": "is"
        },
        {
            "count": 36022,
            "name": "of"
        },
        {
            "count": 9940,
            "name": "that"
        },
        {
            "count": 63146,
            "name": "the"
        },
        {
            "count": 25647,
            "name": "to"
        },
        {
            "count": 9625,
            "name": "was"
        }
    ]
}
```

#### Understanding the example

Working backwards, the output JSON file has two high-level fields:

```
    "typename": "Suffix.SortedWords",
```

This field tells consumers that this JSON file contains a list of sorted words that is suitable for finding suffixes. Consumers that expect this type of data can check that `typename` is "Suffix.SortedWords".

> **NOTE:** The name "Suffix.SortedWords" is arbitrary. It can be anything that uniquely describes the data.

```
    "words": [
        {
            "count": 21970,
            "name": "a"
        },
        ...
```

This field contains the sorted word list as a JSON array of objects representing words. Each word object contains the word itself ("name") and its frequency ("count").

> **NOTE:** This JSON data can have a different structure, as long as it can easily be parsed by C++ and Python code into an ordered data structure. For example this format would also work:
>
> ```
>    "words": {
>        "a": 21970,
>        "and": 27624,
>        ...
>    }
> ```
>
> Where words are the keys of a JSON object. One could argue that because it is missing the "name" and "count" labels, this alternative format is less understanble than the JSON used in this example.

Digging into the example code, it first parses lines in a DX1 formatted file into word structures:

```c++
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
```

Then it sorts the words in a way that is suitable for finding suffixes (lexicographically). Sorting is achieved using the `<` operator defined on `struct word` (see above):

```c++
int transform_data(std::vector<struct word> &words) {
  // Sort the word list.
  std::sort(words.begin(), words.end());
  return 0;
}
```

Finally it constructs a JSON object using the sorted vector of words and writes that object to a file:

```c++
int write_output(const char *output_filename, std::vector<struct word> &words) {
  // ... omitted for clarity

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
    // ... omitted for clarity

  return 0;
}
```

To better understand the JSON operations, see [Reading and Writing to JSON files](./Example4.md).

[Go to Example 2](./Example2.md)

[Back to Table of Contents](../README.md#table-of-contents)
