## Example Plan of Attack

1. Establish input and output data types for steps in the Crab algorithm. For example, to convert `MainWindow::read_dx1_file` to an independent step, we defined the `typename` "Suffix.SortedWords" and its associated JSON format in a precise way. This serves as initial documentation for each step.

2. Determine if any steps can be executed in parallel. If so, write this in high-level control flow documentation. This will become valuable if user demand performance improvements or support for running in the Cloud.

3. Begin converting each step in the Crab algorithm so that it can read its input from a set of files and write its output as JSON to a file, beginning with the C++ code available in [QtLing](https://github.com/edahlgren/QtLing). It will likely be easiest to start at the beginning of the Crab algorithm (from reading DX1 formatted files), so that it's easy to develop test data from interesting inputs like [browncorpus.dx1](https://github.com/edahlgren/QtLing/blob/master/QtLing/browncorpus.dx1).

4. Following from step 3, as each step is converted, collect test data (input files and expected output files).

5. Integrate the new code and the test data into a series of scripts that run on [TravisCI](https://travis-ci.org/) or any other free-to-use continuous integration system. Make sure that scripts are also easy to run locally for manually testing.

6. Continue writing documentation and porting steps in the Crab algorithm to Python.

7. Convert the GUI to read linguistic data from the filesystem and to subscribe for progress updates over a connection (pipe or socket).

The ordering of these steps can change, but there is value in doing steps 1 and 2 first: they make parallel development in C++ and Python much easier by establishing common data formats and rules of execution.

[Back to Table of Contents](../README.md)
