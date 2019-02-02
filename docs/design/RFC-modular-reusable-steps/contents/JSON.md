## JSON Object Format

In the first three examples presented in the [Examples](./Examples.md) section, a JSON object is written to a file. Each JSON object contains a different type of linguistic data, but all objects share an important field in common, a `typename`. The `typename` field is a label representing the type of data contained in the JSON object. 

From example 1 (`MainWindow::read_dx1_file`):

```
{
    "typename": "Suffix.SortedWords",
     ...
}
```

From example 2 (`CLexicon::step1_from_words_to_protostems`):

```
{
    ...
    "typename": "Suffix.Protostems"
}
```

From example 3 (`CLexicon::step1_from_words_to_protostems`):

```
{
    ...
    "typename": "Suffix.ProtostemRanges",
    ...
}
```

A JSON object does not strictly need to contain the `typename` field. But having this field is very useful, especially for a set of programs that depend on reading *specific* types of linguistic data, like the examples presented in this proposal.

It is recommended to use a similar convention to validate that input at each step of the Crab algorithm is valid and expected. After all, it is very easy to accidently pass the wrong file to a program. This convention allows programs to fail gracefully when this happens.

[[Back to Table of Contents]](../README.md#table-of-contents)
