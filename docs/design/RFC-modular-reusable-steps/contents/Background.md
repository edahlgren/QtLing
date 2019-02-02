## Background

As of February 2019, [QtLing](https://github.com/edahlgren/QtLing) holds much of its state in two high-level C++ classes:

+ `MainWindow`: This class is used to control the GUI (graphical user interface) and execute the main algorithm. See [mainwindow.h](https://github.com/edahlgren/QtLing/blob/6df4bf4898274a26db7fc961f4cc7e8f7c0a91eb/QtLing/mainwindow.h) and [mainwindow.cpp](https://github.com/edahlgren/QtLing/blob/6df4bf4898274a26db7fc961f4cc7e8f7c0a91eb/QtLing/mainwindow.cpp) for more details.

+ `CLexicon`: This class is used to store word sets, metadata about words (e.g. stems, affixes, signatures), and temporary data structures. The main algorithm in [QtLing](https://github.com/edahlgren/QtLing) (the Crab algorithm) is defined as a set of methods on this class. See [Lexicon.h](https://github.com/edahlgren/QtLing/blob/6df4bf4898274a26db7fc961f4cc7e8f7c0a91e/QtLing/Lexicon.h) and [mainwindow.cpp](https://github.com/edahlgren/QtLing/blob/6df4bf4898274a26db7fc961f4cc7e8f7c0a91e/QtLing/lexicon_crab1.cpp) for details.

There exists a circular dependency between `MainWindow` and `CLexicon` (see [Lexicon.h](https://github.com/edahlgren/QtLing/blob/6df4bf4898274a26db7fc961f4cc7e8f7c0a91e/QtLing/Lexicon.h#L202) and [mainwindow.h](https://github.com/edahlgren/QtLing/blob/6df4bf4898274a26db7fc961f4cc7e8f7c0a91e/QtLing/mainwindow.h#L116)). This circular dependency makes it relatively difficult to split the logic of [QtLing](https://github.com/edahlgren/QtLing) into smaller, modular components. This is in part due to the fact that steps of the Crab algorithm use the `MainWindow` class to show progress updates in the GUI.

For example, in `MainWindow::MainWindow`, a `CLexicon` object stores a pointer to the `MainWindow`'s progress bar:

```c++
MainWindow::MainWindow()
{
    // ... omitted for clarity

    get_lexicon()->set_progress_bar (m_ProgressBar);

    // ... omitted for clarity
}
```

The `CLexicon` class uses this pointer to the `MainWindow`'s progress bar in `CLexicon::step1_from_words_to_protostems`, the first step of the Crab algorithm:

```c++
void CLexicon::step1_from_words_to_protostems() {
    // ... omitted for clarity

    m_ProgressBar->reset();
    m_ProgressBar->setMinimum(0);
    m_ProgressBar->setMaximum(Words->size());
    m_StatusBar->showMessage("1. Find proto-stems.");
    m_Parses->clear();

    // ... omitted for clarity
}
```

Overall, the Crab algorithm is split into two phases: `CLexicon::Crab1` and `CLexicon::Crab2`. In each phase, intermediate steps are almost entirely side-effectual, meaning that intermediate steps mostly read and write to fields in the `CLexicon` class instead of using temporary state variables. 

Because the `CLexicon` class maintains so much state, it is relatively difficult to recreate only the state necessary to test an individual step of the Crab algorithm.

[Back to Table of Contents](../README.md)
