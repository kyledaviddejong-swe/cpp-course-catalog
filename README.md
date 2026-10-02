# C++ Course Catalog · Binary Search Tree

A command-line course catalog that loads course records from a file, prints them in alphanumeric order, and searches by course number to display a course and its prerequisites. The central data structure is a binary search tree implemented directly in C++.

**Available here:** runnable source, a small sample dataset, and regression tests. No third-party C++ libraries are required.

## What the project demonstrates

- Building, searching, traversing and freeing a binary search tree.
- Parsing course records and checking that referenced prerequisites exist.
- Keeping the current catalog intact when a replacement file fails validation.
- Separating file loading, tree operations, output and menu handling into functions.

The tree was chosen because the program needs both lookup by course number and sorted output. In-order traversal provides the sorted list without a separate sorting step. The tradeoff is that this tree is not self-balancing, so insertion order affects its performance.

## Build and run

Use a C++17-capable compiler. Run these commands from the repository root.

**Linux or macOS with GCC/Clang available:**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic ProjectTwo.cpp -o catalog
./catalog
```

`clang++` can be used in place of `g++`.

**Windows with MinGW-w64 g++ available in PowerShell:**

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic ProjectTwo.cpp -o catalog.exe
.\catalog.exe
```

Choose `1` and enter `data/courses.csv`. Choose `2` to list all courses, `3` to search, or `9` to exit. Searching for `cs410` displays:

```text
CS410: Software Project
Prerequisites:
CS350: Database Applications
CS260: Computer Systems
```

Searches ignore ASCII letter case and surrounding whitespace. Closing standard input also exits the program.

## Input format

Each nonblank line contains a course number, a title, and zero or more prerequisite course numbers. There is no header row.

```text
CS120,Programming Foundations
CS240,Data Structures,CS120
```

- Course numbers must be unique after case normalization, and course numbers and titles must be nonempty.
- Prerequisites must refer to a course somewhere in the same file. They do not have to appear earlier in the file.
- Blank lines and surrounding whitespace are ignored. Both LF and CRLF line endings are accepted.
- Empty optional prerequisite fields are ignored.
- This is a simple comma-delimited format. Quoted fields and commas within titles are not supported.
- If the supplied filename cannot be opened, the program also tries it with `.csv` appended.

Try the six-course example in [data/courses.csv](data/courses.csv).

## Design and limits

For N courses, P total prerequisite references, and tree height h:

| Operation | Cost |
|---|---|
| Insert or look up one course | O(h); O(log N) when the tree is reasonably balanced, O(N) in the worst case |
| Print every course in order | O(N) |
| Validate duplicate IDs and prerequisite references | O(N² + PN), using vector scans |
| Build the tree after validation | O(N log N) for a reasonably balanced result, O(N²) in the worst case |
| Stored course records and prerequisite references | O(N + P), excluding the lengths of the strings |

Printing one course's prerequisite details also looks up each prerequisite in the tree. The implementation uses recursive tree operations, with stack depth proportional to h. It is intended for small catalogs, not very large or adversarial datasets.

A sorted vector with binary search would also be a reasonable choice for a mostly read-only catalog. This implementation retains the explicit BST to demonstrate the data structure. It checks prerequisite existence, but does not detect prerequisite cycles or build a degree plan.

## Regression checks

The tests require Python 3's standard library in addition to the compiled program.

```bash
python3 tests/test_catalog.py ./catalog
```

On Windows, use `python tests/test_catalog.py .\catalog.exe`.

The nine tests cover sorted output, prerequisite lookup, line endings and whitespace, invalid menu input, EOF at each prompt, rejected files, and successful and unsuccessful catalog replacement. The program was compiled and tested on Linux with GCC; Windows and macOS commands have not been independently tested here.

## Background

My implementation covers file loading, the binary search tree, traversal, search, and the console interface. The design exercise compared vectors, hash tables, and binary search trees before selecting an approach.

Developed as undergraduate coursework. See [credits and acknowledgments](CREDITS.md).
