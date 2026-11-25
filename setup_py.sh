#!/usr/bin/env bash
# Sets up the Python environment for the graph analysis: creates a virtual
# environment in .venv/ at the project root (unless one is already there) and
# installs the packages listed in requirements.txt into it.
#
# Run from anywhere:
#     ./setup_py.sh
# Then activate the environment:
#     source .venv/bin/activate
#
# Running it again is safe: it reuses .venv/ and installs anything that has
# been added to requirements.txt since.

set -euo pipefail

# Work from the project root (the directory this script is in)
cd "$(dirname "$0")"

if [ ! -d .venv ]; then
    python3 -m venv .venv
fi
venv/bin/python -m ensurepip --upgrade
venv/bin/python -m pip install --upgrade pip
.venv/bin/python -m pip install -r requirements.txt

echo "Done. Activate the environment with: source .venv/bin/activate"
