#!/usr/bin/env bash
# Sets up the whole project in one go:
#   1. the Python environment for the analysis notebook, in .venv/
#      (done by setup_py.sh, from requirements.txt)
#   2. the C++ driver, ./wait_for_graph (done by the Makefile)
#
# Run from anywhere:
#     ./setup.sh
#
# Running it again is safe: both steps only redo what has changed. It stops at
# the first step that fails.

set -euo pipefail

# Work from the project root (the directory this script is in)
cd "$(dirname "$0")"

echo "== Python environment =="
./setup_py.sh

echo "== Driver =="
make

echo "Setup complete: .venv/ is ready and ./wait_for_graph is built."
