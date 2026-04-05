#!/bin/bash
# Helper script to launch the Python-to-C Transpiler UI

# Ensure we're in the right directory
CDIR="$(cd "$(dirname "$0")" && pwd)"
cd "$CDIR"

# Export the root path so bridge.py can find py2c
export PYTHONPATH="$CDIR/ui:$PYTHONPATH"

echo "Starting Streamlit UI..."
streamlit run ui/app.py
