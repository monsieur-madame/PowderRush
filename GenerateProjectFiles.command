#!/bin/bash
# Generate Xcode project files for PowderRush
# Works on any Mac - auto-detects UE5 installation path

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
UPROJECT="$SCRIPT_DIR/PowderRush.uproject"

if [ ! -f "$UPROJECT" ]; then
    echo "Error: PowderRush.uproject not found at $UPROJECT"
    exit 1
fi

# Read engine association from .uproject
ENGINE_VERSION=$(python3 -c "import json; print(json.load(open('$UPROJECT'))['EngineAssociation'])" 2>/dev/null || echo "")

if [ -z "$ENGINE_VERSION" ]; then
    echo "Error: Could not read EngineAssociation from .uproject"
    exit 1
fi

echo "Looking for UE $ENGINE_VERSION..."

# Search common UE5 installation paths on macOS
SEARCH_PATHS=(
    "$HOME/Documents/Epic Games/UE_$ENGINE_VERSION"
    "$HOME/Documents/Unreal Engine/UE_$ENGINE_VERSION"
    "/Users/Shared/Epic Games/UE_$ENGINE_VERSION"
    "/Applications/Epic Games/UE_$ENGINE_VERSION"
    "/opt/UnrealEngine/UE_$ENGINE_VERSION"
)

ENGINE_DIR=""
for path in "${SEARCH_PATHS[@]}"; do
    if [ -d "$path/Engine" ]; then
        ENGINE_DIR="$path"
        break
    fi
done

# If not found in common paths, try to find via Spotlight
if [ -z "$ENGINE_DIR" ]; then
    echo "Searching via Spotlight..."
    FOUND=$(mdfind "kMDItemFSName == 'UE_$ENGINE_VERSION'" -onlyin /Users -onlyin /Applications -onlyin /opt 2>/dev/null | head -1)
    if [ -n "$FOUND" ] && [ -d "$FOUND/Engine" ]; then
        ENGINE_DIR="$FOUND"
    fi
fi

if [ -z "$ENGINE_DIR" ]; then
    echo "Error: Could not find UE $ENGINE_VERSION installation."
    echo "Searched:"
    for path in "${SEARCH_PATHS[@]}"; do
        echo "  $path"
    done
    echo ""
    echo "Set UE_ROOT environment variable to your UE installation, e.g.:"
    echo "  UE_ROOT=/path/to/UE_$ENGINE_VERSION $0"
    exit 1
fi

# Allow override via environment variable
if [ -n "$UE_ROOT" ]; then
    ENGINE_DIR="$UE_ROOT"
fi

GENERATE_SCRIPT="$ENGINE_DIR/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh"

if [ ! -f "$GENERATE_SCRIPT" ]; then
    echo "Error: GenerateProjectFiles.sh not found at $GENERATE_SCRIPT"
    exit 1
fi

echo "Using engine at: $ENGINE_DIR"
echo "Generating Xcode project files..."
echo ""

"$GENERATE_SCRIPT" -project="$UPROJECT" -game -xcode

echo ""
echo "Done! Open PowderRush (Mac).xcworkspace or PowderRush (IOS).xcworkspace"
