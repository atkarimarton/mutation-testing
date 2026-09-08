# Mutation Testing Using the GCC Compiler's Plugin API

A GCC plugin that performs **mutation testing on C source code** by rewriting a function's internal representation (GCC's GENERIC tree) at compile time, then running the project's test suite to see whether the mutation is detected.

This project was built as part of a diploma thesis on mutation testing.

## What is mutation testing?

Mutation testing measures how good a test suite actually is, rather than just how much code it covers. The tool introduces small, deliberate faults ("mutants") into the source — e.g. flipping `<` to `>`, or `&&` to `||` — and re-runs the tests:

- If a test fails → the mutant is **killed** (good, your tests caught the bug).
- If all tests still pass → the mutant **survives** (bad, it's a blind spot in your test suite).

The ratio of killed to total mutants is the **mutation score**, a stronger signal of test quality than line/branch coverage.

## How it works

Instead of textually patching the `.c` file, the plugin hooks into GCC's compilation pipeline (`plugin/plugin.cpp`) and, after parsing, walks the **GENERIC** tree of a target function looking for a specific node type (e.g. a `LT_EXPR`). Each occurrence in the function is numbered by *position*; the plugin is told which `(function, rule, position)` triple to mutate via plugin arguments, swaps that one node for the mutated operator, and lets GCC continue compiling as normal. Only one mutation is ever applied per compilation, which is what allows each mutant to be built and tested in isolation.

The driver scripts (`generate_mutants.sh`, `unity_test.sh`) enumerate this space automatically: for every function in `src/program.c`, for every rule in `rules.txt`, they keep incrementing `position` until the plugin reports there's no more matching node at that index, which is how the total mutant count is discovered without needing to know it up front.

## Prerequisites

- Linux, with GCC and its plugin headers (the plugin is built against GCC's internal `tree.h`/`gcc-plugin.h` API, so the installed GCC must have plugin development support, e.g. `gcc-plugin-dev` on Debian/Ubuntu).
- `g++` to compile the plugin itself.
- GNU `time` (used by the makefile to report elapsed run time).

## Building

```bash
make plugin.so
```

This compiles `plugin/plugin.cpp` against the active GCC's plugin headers (auto-detected via `gcc -print-file-name=plugin`) into a shared object the rest of the tooling loads with `-fplugin=./plugin.so`.

## Usage

| Command | What it does |
|---|---|
| `make unity_test` | Runs the full mutation testing loop against the [Unity](http://www.throwtheswitch.org/unity) test suite in `test/`. For every mutant it recompiles, links against the tests, and reports `MUTANT CAUGHT` or `MUTANT SURVIVED`, finishing with the overall mutation score. |
| `make generate_mutants` | Generates and compiles every possible mutant *without* running the tests — useful for sanity-checking how many mutants exist for the current source and rule set. |
| `make generate_report` | Builds an interactive `test_report.html` from the mutants left behind in `result/`, showing the original source with surviving mutants highlighted inline (see below). |
| `make clean` | Removes build artifacts (`*.o`, `*.so`, `*.out`, `*.html`, `tmp/`, `result/`). |

### Typical workflow

```bash
make unity_test        # mutate + test, prints mutation score to the console
make generate_report    # turn the surviving mutants into a browsable HTML report
open test_report.html   # inspect which lines your tests failed to cover
```

`unity_test.sh` deletes the generated mutant file for every mutant that was caught, so only **surviving** mutants remain in `result/` afterwards — which is exactly what `generate_report` visualizes.

## Mutation operators

Enabled operators are listed one per line in `rules.txt`; comment out or remove a line to disable that operator.

| Category | Operators |
|---|---|
| Logical | `and_to_or`, `or_to_and` |
| Relational | `lt_to_gt`, `lt_to_le`, `lt_to_ge`, `le_to_lt`, `le_to_gt`, `le_to_ge`, `gt_to_lt`, `gt_to_ge`, `gt_to_le`, `ge_to_gt`, `ge_to_lt`, `ge_to_le` |
| Arithmetic | `plus_to_minus`, `minus_to_plus`, `div_to_mul`, `mul_to_div` |
| Equality | `eq_to_ne`, `ne_to_eq` |
| Conditional | `truthify` (force condition to always true), `falsify` (force condition to always false) |
| Return | `return_zero` (force a function to return `0`) |

## Example

`src/program.c` ships a small example function:

```c
int calc(int a, int b) {
    if (a < 2) {
        return (a+b) * -1;
    } else {
        return a+b;
    }
}
```

with a Unity test suite in `test/programTest.c`. Running `make unity_test` mutates `calc` (e.g. `lt_to_ge` turns `a < 2` into `a >= 2`) and checks whether `test_calc` / `test_greater` catch each change.

## Adapting it to your own code

The tooling currently targets a single hardcoded source file:

1. Replace `src/program.c` (and `src/program.h`) with your own code, or update `SOURCE_FILES` in the `makefile` to point at it.
2. Replace `test/programTest.c` with Unity tests for your code (the Unity framework itself is vendored in `test/`, no separate install needed).
3. Adjust `rules.txt` to enable only the mutation operators relevant to your code.

## Project structure

```
.
├── makefile                    # build/run entry points (plugin.so, unity_test, generate_mutants, generate_report, clean)
├── generate_mutants.sh          # enumerates and compiles every mutant, without testing
├── unity_test.sh                # enumerates every mutant, tests it, prints the mutation score
├── rules.txt                    # enabled mutation operators, one per line
├── plugin/
│   ├── plugin.cpp                # the GCC plugin: GENERIC-tree mutators + report hooks
│   └── plugin.h
├── src/
│   ├── program.c                 # example program under test
│   └── program.h
├── test/
│   ├── programTest.c             # Unity tests for program.c
│   └── unity.c / unity.h / unity_internals.h   # vendored Unity test framework
└── html_report/
    └── report_template.html      # template used to render test_report.html
```

## HTML report

`make generate_report` produces `test_report.html`: the original source of each function, with any mutant that survived testing rendered as a clickable "Show uncaught mutant" dropdown directly under the affected line, showing the mutated line and which operator produced it. This makes it easy to see, at a glance, exactly which branches/conditions/operators are under-tested.
