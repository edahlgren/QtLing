# RFC: Modular reusable steps in QtLing

This document proposes a way to refactor the code in [QtLing](https://github.com/edahlgren/QtLing) so users can easily understand and experiment with the dataflow of its main algorithm (the "Crab" algorithm). The proposal presented is inspired by principles used in [Apache Beam](https://beam.apache.org/get-started/beam-overview/) and is sensitive to debugging, testing, and multi-language support (particularly for C++ and Python). At the core of the proposal is changing each step in the Crab algorithm to output linguistic data to a file on the user’s filesystem.


## Table of Contents

+ [Introduction](#introduction)
+ [Background](./contents/Background.md)
+ [Summary of the Proposal](./contents/Summary.md)
+ [Inspiration](./contents/Inspiration.md)
+ [Examples](./contents/Examples.md)
  + [MainWindow::read_dx1_file](./contents/Example1.md)
  + [CLexicon::step1_from_words_to_protostems (first half)](./contents/Example2.md)
  + [CLexicon::step1_from_words_to_protostems (second half)](./contents/Example3.md)
  + [Reading and Writing to JSON files](./contents/Example4.md)
+ [Testing & Debugging](./contents/TestingDebugging.md)
+ [GUI](./contents/GUI.md)
+ [JSON Objects](./contents/JSON.md)
+ [Example Plan of Attack](./contents/PlanOfAttack.md)

## Introduction

This document proposes a way to refactor the [QtLing](https://github.com/edahlgren/QtLing) project code so users can easily understand and experiment with the dataflow of its main algorithm. In addition to this basic requirement, this proposal was written with three other benefits for users in mind:

+ **Ease of debugging.** This proposal should allow users to inspect the linguistic data produced by steps of the Crab algorithm (e.g. sorted word lists, protostems, affixes, signatures). Additionally, when debugging or profiling a specific step of the Crab algorithm, this proposal should make it possible to execute just that step, saving time and avoiding potential bugs in previous steps.

+ **Single test suite.** This proposal should make it easy to maintain a single test suite that validates code written in both C++ and Python. This is important for keeping C++ and Python code logically in sync while encouraging experimentation.

+ **Multi-language dataflow.** This proposal should give users the opportunity to combine steps of the Crab algorithm written in different languages (e.g. C++, Python). This gives users more flexibility when experimenting and more freedom to reuse code.

While these requirements are arguably the most important in the short-term, this proposal is also sensitive to future features like:

+ **Web support.** For users who are not at all technical, a web application can provide an easy way (no download or setup required) to load data, run a computation, and download results. Additionally, modern desktop applications are being developed more and more frequently with web-centric frameworks like [Electron](https://electronjs.org/) so desktop and web application code can be shared.

+ **Cloud support.** Services like [Cloud Dataflow](https://cloud.google.com/dataflow/) allow users to do intensive data processing on a remote cluster of machines (effectively in the background), often faster than if users ran the same algorithms locally. While strictly not necessary, these services help users visualize the dataflow of their algorithms, for example as directed acyclic graphs.
