#!/bin/bash
# Run script for Einstein Game

# Activate Python environment
if [ -d "venv" ]; then
    source venv/bin/activate
fi

# Change to install directory
cd install/bin 2>/dev/null || cd bin 2>/dev/null || true

# Run game
if [ -f "EinStein_Game" ]; then
    ./EinStein_Game "$@"
elif [ -f "EinStein_Game.exe" ]; then
    ./EinStein_Game.exe "$@"
else
    echo "Game executable not found!"
    exit 1
fi
