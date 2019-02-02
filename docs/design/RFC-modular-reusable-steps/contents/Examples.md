## Examples

This section contains several examples of how to change steps in the Crab algorithm to fit this format:

![Alt text](./format_tmp.svg)

The examples are written in C++ and should be easy to read, build, and execute. They use only C++ standard libraries and a single, header-only third-party library for working with JSON.

#### Setup steps

(Pre-install the C++ json library)

```
$ git clone git@github.com:nlohmann/json.git
$ sudo cp -r json/single_include/nlohmann /usr/local/include
```

(Download the example code)

```
$ git clone git@github.com:edahlgren/QtLing.git
$ cd docs/design/RFC-modular-reusable-steps
```

#### Examples

+ [Example 1: MainWindow::read_dx1_file](./Example1.md)
+ [Example 2: CLexicon::step1_from_words_to_protostems (first half)](./Example2.md)
+ [Example 3: CLexicon::step1_from_words_to_protostems (second half)](./Example3.md)
+ [Example 4: Reading and Writing to JSON files](./Example4.md)


[Back to Table of Contents](../README.md#table-of-contents)
