# Wait-For Graph and Deadlock Detection

Given a set of transactions that run concurrently, this project works out which
transactions end up waiting for each other's locks, draws that as a wait-for
graph, and reports the deadlocks (the cycles in the graph).

A C++ program executes the transactions and writes the schedule and the graph
to files. A Jupyter notebook runs that program, reads the files back, draws the
graph and finds the cycles.

## Install

You need:

- a C++17 compiler available as `c++`, and `make`
- Python 3.9 or newer

Developed with Apple clang 21 and Python 3.9.6 on macOS.

From the project root, run:

```bash
./setup.sh
```

This does two things, and is safe to run again:

1. Creates a Python virtual environment in `.venv/` and installs the packages
   in `requirements.txt` (`networkx`, `matplotlib`, `ipykernel`).
2. Builds the driver, `./wait_for_graph`, with `make`.

To do the steps separately, run `./setup_py.sh` for the Python environment and
`make` for the driver.

## Verify the sample experiments

The three inputs in `tests/` are the test cases from `sample outputs.pdf`
(test case 1 is also the example in `Project2.pdf`). There is no automated
pass/fail check: you run the samples and compare the results with the PDF.

### In the notebook

1. Open `analyze_graph.ipynb` in VS Code, or another Jupyter front end, and
   choose the `.venv` environment as the kernel. `requirements.txt` installs
   the kernel only, not a Jupyter server.
2. Run the first two cells (imports and helpers), then the cell under
   **Given Sample Input Checks**.

For each test case the notebook prints the input, Output 1 (the round-robin
schedule), Output 2 (the wait-for graph, drawn with the blocked operation on
every edge) and Output 3 (the cycles). You should see:

| Test | Edges of the wait-for graph | Output 3 |
|---|---|---|
| 1 | T1→T2 `T1:W(b,1)`, T2→T1 `T2:W(a,2)`, T2→T3 `T2:W(a,2)`, T3→T2 `T3:W(c,3)` | Cycle 1: T1 to T2 (or T2 to T1)<br>Cycle 2: T2 to T3 (or T3 to T2) |
| 2 | T1→T2 `T1:W(e,8)`, T2→T1 `T2:R(h)` | Cycle 1: T1 to T2 (or T2 to T1) |
| 3 | none | No Cycle. |

The cell under **Custom Input Check** stops with an error until you set
`custom_input_path` to a file of your own, so "Run All" ends there by design.

### From the command line

```bash
make samples
```

This runs the driver on every `tests/testN.txt` and writes, for each one:

- `output_schedules/out_N.txt`: the round-robin schedule (Output 1)
- `graphs/wait_for_graph_N.graphml`: the wait-for graph (Output 2)

The command line stops there. Finding the cycles (Output 3) is done by the
notebook.


### Running your own input

An input file has one transaction per line, starting on the first line. Each
transaction is a list of reads `R(x)` and writes `W(x,value)`, where `x` is a
record named by one lowercase letter, ending in a commit `C`:

```
T1: R(a); W(b,1); C
T2: R(b); R(c); W(a,2); C
T3: R(a); W(c,3); C
```

Transactions are numbered T1, T2, ... by their position in the file. Either set
`custom_input_path` in the notebook's **Custom Input Check** cell, or run the
driver yourself with a run number `N` that names the output files:

```bash
./wait_for_graph 4 < my_input.txt
```

Run the driver from the project root: it writes to `output_schedules/` and
`graphs/` relative to the directory it is run from.

## Architecture

```
tests/testN.txt
      │  stdin
      ▼
 ┌──────────┐     ┌──────────────┐     output_schedules/out_N.txt
 │ ingester │────▶│ WaitForGraph │────▶
 └──────────┘     └──────────────┘     graphs/wait_for_graph_N.graphml
   parses the       executes the                    │
   transactions     schedule under locks            ▼
                                            analyze_graph.ipynb
   └──────── ./wait_for_graph (C++) ───┘    draws the graph, finds cycles
```

### C++ driver

| File | Role |
|---|---|
| `include/operation.h` | `TransactionOperation`: one read, write or commit of one transaction. |
| `include/ingester.hpp`, `src/ingester.cpp` | `TransactionList`: parses the input into one `Schedule` per transaction and prints the round-robin ordering. |
| `include/graph.h` | `Node` and `Graph`: a directed graph whose edges carry a text label. |
| `include/graph_maker.hpp`, `src/graph_maker.cpp` | `WaitForGraph`: executes the schedule against a lock table, builds the graph and writes it as GraphML. |
| `src/driver.cpp` | `main`: reads stdin, then writes the schedule file and the graph file for run `N`. |

`WaitForGraph` executes the transactions in round-robin passes, one operation
per transaction per pass, with these rules:

- A read needs a shared lock: it waits if another transaction holds a write
  lock on the record.
- A write needs an exclusive lock: it waits if another transaction holds any
  lock on the record.
- Locks are held until the transaction commits.
- A transaction that has to wait gets an edge to every transaction holding a
  lock in its way, labelled with the blocked operation, and retries on the next
  pass.
- When a transaction commits, its locks are released and the edges pointing at
  it are removed.

Execution ends when every transaction has committed, or when a whole pass goes
by with no transaction able to move. In the second case the edges left in the
graph are the deadlock. The graph therefore holds only the waits that were
never resolved, which is why test case 3 has no edges.

The graph file has one node per transaction (`T1`, `T2`, ...) and a string
attribute `transaction_op` on every edge, such as `T2:W(a,2)`.

### Analysis notebook

`analyze_graph.ipynb` does not build the driver; it expects `./wait_for_graph`
to exist. For each input it runs the driver, reads the schedule file and loads
the GraphML file with `networkx`. Cycles are found with
`networkx.simple_cycles`, so deadlock detection lives in the notebook, not in
the C++ code. The graph is drawn with `matplotlib`.

### Build and setup files

| File | Role |
|---|---|
| `Makefile` | `make` builds the driver, `make samples` runs it on the sample inputs, `make clean` removes the build. |
| `setup.sh` | Runs `setup_py.sh`, then `make`. |
| `setup_py.sh` | Creates `.venv/` and installs `requirements.txt`. |
