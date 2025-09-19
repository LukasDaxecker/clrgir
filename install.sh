#!/bin/bash

gcc clrgir.c -o clrgir

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

mkdir -p ~/bin
mv clrgir ~/bin/

# Add ~/bin to PATH permanently if not already added
SHELL_CONFIG="$HOME/.bashrc"
if ! grep -q 'export PATH="$HOME/bin:$PATH"' "$SHELL_CONFIG"; then
    echo 'export PATH="$HOME/bin:$PATH"' >> "$SHELL_CONFIG"
    echo "Added ~/bin to PATH in $SHELL_CONFIG. Please restart your terminal or run 'source $SHELL_CONFIG'."
fi

echo "Finished!"
