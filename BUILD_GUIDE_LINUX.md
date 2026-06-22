# Building Discord RPC Plugin for Qt Creator 19.0.2 - Linux Guide

## Prerequisites

### 1. Verify Your Setup

```bash
# Check Qt Creator
qtcreator --version

# Should output something like:
# Qt Creator Version: 19.0.2
# Qt Version: 6.11.1

# Check CMake
cmake --version
# Should be 3.21 or newer

# Check C++ compiler
g++ --version
# or clang++ --version
```

### 2. Install Build Tools

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    qt6-base-dev \
    qt6-declarative-dev
```

#### Fedora

```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install \
    cmake \
    git \
    qt6-qtbase-devel \
    qt6-qtdeclarative-devel
```

#### Arch

```bash
sudo pacman -Syu
sudo pacman -S base-devel cmake git qt6-base
```

## Step 1: Locate Qt Creator Installation

Find where Qt Creator is installed:

```bash
which qtcreator
# Output: /usr/bin/qtcreator (or /home/user/Qt/Tools/QtCreator/bin/qtcreator)

# Find the actual Qt Creator root
QTCREATOR_BIN=$(which qtcreator)
QTCREATOR_ROOT=$(dirname "$(dirname "$QTCREATOR_BIN")")
echo "Qt Creator root: $QTCREATOR_ROOT"

# Verify essential files exist
ls "$QTCREATOR_ROOT"/lib/qtcreator/plugins/
ls "$QTCREATOR_ROOT"/src/
```

## Step 2: Build discord-rpc Library

```bash
git clone https://github.com/discordapp/discord-rpc.git
cd discord-rpc
mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# Install to a known location
DISCORD_RPC_INSTALL=$HOME/.local/opt/discord-rpc
cmake --install . --prefix "$DISCORD_RPC_INSTALL"

# Verify installation
ls "$DISCORD_RPC_INSTALL"/include/
ls "$DISCORD_RPC_INSTALL"/lib/
```

## Step 3: Set Up Environment Variables

Create a `setup-build.sh` file:

```bash
#!/bin/bash

# Find Qt Creator automatically
QTCREATOR_BIN=$(which qtcreator)
if [ -z "$QTCREATOR_BIN" ]; then
    echo "Error: Qt Creator not found in PATH"
    exit 1
fi

export QTCREATOR_ROOT=$(dirname "$(dirname "$QTCREATOR_BIN")")
export QT_VERSION=6.11.1
export QT_PATH=$(find /opt/Qt /usr/lib /usr/local -name "6.11.1" -type d 2>/dev/null | head -1)

if [ -z "$QT_PATH" ]; then
    # Try Qt installed via package manager
    export QT_PATH=$(pkg-config --variable=libdir Qt6Core | sed 's|/lib$||')
fi

export DISCORD_RPC_PATH=$HOME/.local/opt/discord-rpc

echo "Build Environment:"
echo "  Qt Creator Root: $QTCREATOR_ROOT"
echo "  Qt Version: $QT_VERSION"
echo "  Qt Path: $QT_PATH"
echo "  Discord RPC Path: $DISCORD_RPC_PATH"

# Verify paths
if [ ! -d "$QTCREATOR_ROOT/src" ]; then
    echo "Warning: Qt Creator source not found at $QTCREATOR_ROOT/src"
    echo "This may cause compilation errors."
fi

if [ ! -d "$DISCORD_RPC_PATH/lib" ]; then
    echo "Error: discord-rpc not found at $DISCORD_RPC_PATH"
    echo "Please build discord-rpc first."
    exit 1
fi
```

Make it executable:

```bash
chmod +x setup-build.sh
source setup-build.sh
```

## Step 4: Create Plugin Project Structure

```bash
mkdir -p DiscordRPCPlugin/{src,resources,build}
cd DiscordRPCPlugin
```

Copy the provided files:
- `CMakeLists.txt`
- `src/discordrpcplugin.h`
- `src/discordrpcplugin.cpp`
- `src/discordrpcmanager.h`
- `src/discordrpcmanager.cpp`
- `src/discordrpcconstants.h`
- `src/DiscordRPCPlugin.json`

## Step 5: Build the Plugin

```bash
cd DiscordRPCPlugin/build

# Load environment
source ../setup-build.sh

# Configure with CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$QTCREATOR_ROOT:$QT_PATH" \
    -DQt6_DIR="$QT_PATH/lib/cmake/Qt6" \
    -DDISCORD_RPC_DIR="$DISCORD_RPC_PATH"

# Build
cmake --build . -j$(nproc)

# Verify the plugin was built
ls -lh DiscordRPCPlugin.so
```

### Troubleshooting Build Errors

**Error: "Could not find Qt6"**

```bash
# Find Qt6 cmake directory
find /opt /usr -name "Qt6Config.cmake" 2>/dev/null

# Use the path found, e.g.:
cmake .. -DQt6_DIR=/opt/Qt/6.11.1/gcc_64/lib/cmake/Qt6
```

**Error: "Could not find QtCreator"**

```bash
# Ensure $QTCREATOR_ROOT/src exists
ls "$QTCREATOR_ROOT"/src/plugins/

# If missing, you need Qt Creator with source code
# Either build from source or use the SDK
```

**Error: "discord-rpc not found"**

```bash
# Verify discord-rpc installation
ls "$DISCORD_RPC_PATH"/include/discord-rpc.h
ls "$DISCORD_RPC_PATH"/lib/libdiscord-rpc.so

# If missing, rebuild discord-rpc:
cd ~/discord-rpc/build
cmake --install . --prefix "$DISCORD_RPC_PATH"
```

## Step 6: Install the Plugin

Qt Creator looks for plugins in several locations. Use the user directory:

```bash
# Create plugin directory
mkdir -p ~/.local/share/QtProject/QtCreator/plugins/19.0.2/

# Copy the plugin
cp build/DiscordRPCPlugin.so ~/.local/share/QtProject/QtCreator/plugins/19.0.2/

# Make sure discord-rpc library is accessible
export LD_LIBRARY_PATH="$DISCORD_RPC_PATH/lib:$LD_LIBRARY_PATH"

# Or copy the discord-rpc library to the plugin directory
cp "$DISCORD_RPC_PATH"/lib/*.so* ~/.local/share/QtProject/QtCreator/plugins/19.0.2/
```

### Alternative: System-wide Installation

```bash
sudo cp build/DiscordRPCPlugin.so \
    /usr/lib/qtcreator/plugins/

sudo cp "$DISCORD_RPC_PATH"/lib/libdiscord-rpc.so \
    /usr/lib/ || /usr/local/lib/

sudo ldconfig
```

## Step 7: Verify Plugin Installation

```bash
# Start Qt Creator with debug output
qtcreator -logging.rules=qtc.extensionsystem=debug 2>&1 | grep -i discord

# In Qt Creator:
# Go to Help > About Plugins
# Search for "DiscordRPC"
# Verify it's enabled
```

If not loading, check the debug output for errors like:
- "Cannot load plugin"
- "Missing dependency"
- "Cannot find libdiscord-rpc.so"

### Fix Missing Library

If you see "Cannot find libdiscord-rpc.so", add to your shell profile:

```bash
# Add to ~/.bashrc or ~/.zshrc
export LD_LIBRARY_PATH="$HOME/.local/opt/discord-rpc/lib:$LD_LIBRARY_PATH"

# Or set library path for Qt Creator specifically
echo 'export LD_LIBRARY_PATH="$HOME/.local/opt/discord-rpc/lib:$LD_LIBRARY_PATH"' >> ~/.profile
```

Then restart your shell and Qt Creator.

## Step 8: Configure Discord Application

1. Visit https://discord.com/developers/applications
2. Create a new application "Qt Creator"
3. Copy the **Client ID**
4. Edit `src/discordrpcmanager.h`:

```cpp
static constexpr const char *DISCORD_CLIENT_ID = "YOUR_CLIENT_ID_HERE";
```

5. Rebuild:

```bash
cd build
cmake --build . -j$(nproc)
cp DiscordRPCPlugin.so ~/.local/share/QtProject/QtCreator/plugins/19.0.2/
```

6. Restart Qt Creator

## Step 9: Test the Plugin

```bash
# Ensure Discord is running
discord &

# Start Qt Creator
qtcreator &

# Open a project
# Check Discord profile for Qt Creator presence
```

## Troubleshooting

### Plugin loads but Discord doesn't update

**Check Discord is running:**
```bash
ps aux | grep -i discord
```

**Enable debug logging in the plugin:**

Edit `src/discordrpcmanager.cpp` and ensure:
```cpp
qDebug() << "Discord RPC: Updating presence...";
```

Then run:
```bash
qtcreator 2>&1 | grep "Discord RPC"
```

### Library path issues at runtime

If Discord RPC doesn't work:

```bash
# Check if libdiscord-rpc.so is found
ldd ~/.local/share/QtProject/QtCreator/plugins/19.0.2/DiscordRPCPlugin.so | grep discord

# If not found, add to LD_LIBRARY_PATH
export LD_LIBRARY_PATH="$HOME/.local/opt/discord-rpc/lib:$LD_LIBRARY_PATH"
qtcreator
```

### Qt version mismatch

If you get "Qt version mismatch" errors:

```bash
# Use the exact Qt version Qt Creator was built with
pkg-config --modversion Qt6Core

# Rebuild with matching version
cmake .. -DQt6_DIR=$(pkg-config --variable=libdir Qt6Core)/../cmake/Qt6
```

## Automating the Build

Create a `build.sh` script:

```bash
#!/bin/bash
set -e

echo "Building Discord RPC Plugin for Qt Creator..."

# Load environment
source setup-build.sh

cd build

# Clean
rm -rf *

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$QTCREATOR_ROOT:$QT_PATH" \
    -DQt6_DIR="$QT_PATH/lib/cmake/Qt6" \
    -DDISCORD_RPC_DIR="$DISCORD_RPC_PATH"

# Build
cmake --build . -j$(nproc)

# Install
PLUGIN_DIR="$HOME/.local/share/QtProject/QtCreator/plugins/19.0.2"
mkdir -p "$PLUGIN_DIR"
cp DiscordRPCPlugin.so "$PLUGIN_DIR/"
cp "$DISCORD_RPC_PATH"/lib/*.so* "$PLUGIN_DIR/" 2>/dev/null || true

echo "Build complete!"
echo "Plugin installed to: $PLUGIN_DIR"
echo "Restart Qt Creator to load the plugin"
```

Make executable:
```bash
chmod +x build.sh
./build.sh
```

## Next Steps

Once working:
1. Add configuration UI
2. Support more file types
3. Add activity tracking
4. Create GitHub releases

Good luck! 🎉
