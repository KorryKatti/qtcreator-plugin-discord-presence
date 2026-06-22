# Discord RPC Plugin for Qt Creator

A Qt Creator plugin that adds Discord Rich Presence support, showing what you're coding in real-time on your Discord profile.

## Features

- **Live Presence** — Shows current project and file in Discord
- **MIME-based Detection** — Recognizes 30+ file types (C++, Python, QML, Rust, Java, etc.)
- **Control Menu** — Start/Stop Discord RPC from Tools menu
- **Session Timer** — Tracks time spent on current file
- **Auto-Update** — Updates every second as you work
- **File Type Icons** — Different Discord icons per language

## What It Shows

Your Discord profile will display:

```
Editing C++ Source File
main.cpp/MyApp
```

- **Details** — What you're doing (Editing C++ Source File)
- **State** — File name and project (main.cpp/MyApp)
- **Large Icon** — File type icon (cxx, python, qml, etc.)
- **Small Icon** — Qt Creator icon with project name
- **Timestamp** — Session duration

## Installation (Pre-built)

Download the latest release for your platform from [Releases](https://github.com/KorryKatti/qtcreator-plugin-discord-presence/releases).

1. Open Qt Creator
2. Go to **Help** > **About Plugins**
3. Click **Install Plugin**
4. Select the downloaded zip file
5. Restart Qt Creator
6. Verify via **Help** > **About Plugins** — search for "DiscordRPC"

## Building from Source

### Prerequisites

- Qt Creator 19.0.2 with development files
- Qt 6.11.1 (or compatible Qt 6.x)
- CMake 3.21+
- C++20 compatible compiler (GCC 12+ / Clang 14+)
- [discord-rpc](https://github.com/discord/discord-rpc) library

### Step 1: Build discord-rpc

```bash
git clone https://github.com/discord/discord-rpc.git
cd discord-rpc
mkdir build && cd build

# Fix duplicate key in .clang-format (remove second IndentCaseLabels line)
# Then build with -fPIC for shared library linking
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_CXX_FLAGS="-Wno-error -fpermissive"

cmake --build . -j$(nproc) --target discord-rpc
cmake --install . --prefix /path/to/discord-rpc-install
```

### Step 2: Build the Plugin

```bash
git clone https://github.com/KorryKatti/qtcreator-plugin-discord-presence.git
cd qtcreator-plugin-discord-presence
mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DDISCORD_RPC_DIR=/path/to/discord-rpc-install

cmake --build . -j$(nproc)
```

### Step 3: Package and Install

```bash
cd plugins
cp /path/to/DiscordRPCPlugin.json libDiscordRPCPlugin.json
zip DiscordRPCPlugin-<platform>.zip libDiscordRPCPlugin.so libDiscordRPCPlugin.json
```

Then install via **Help** > **About Plugins** > **Install Plugin**.

## Discord Application Setup

The plugin uses a shared Discord Application ID (`937400240473006092`). You can use this directly — no setup needed.

If you want your own application:

1. Go to [Discord Developer Portal](https://discord.com/developers/applications)
2. Create a new application
3. Copy the **Client ID**
4. Edit `discordrpcconstants.h`:

```cpp
constexpr const char DISCORD_CLIENT_ID[] = "YOUR_CLIENT_ID_HERE";
```

5. Rebuild and reinstall the plugin

## Supported File Types

| Category | Extensions |
|----------|-----------|
| C/C++ | .cpp, .hpp, .cxx, .hxx, .cc, .hh, .c, .h, .i |
| Qt/QMake | .pro, .pri, .ui, .qrc, .qml, .qss |
| Python | .py |
| JavaScript/TypeScript | .js, .ts |
| Web | .html, .css, .scss, .json, .xml |
| Other Languages | .rb, .rs, .lua, .java, .asm, .cs |
| Build | CMakeLists.txt, Makefile, .gitignore |
| Text | .txt, .md |

## Architecture

```
├── discordrpcplugin.h/cpp       # Plugin entry point (IPlugin)
├── discordrpcmanager.h/cpp      # Discord RPC core logic
├── discordrpcconstants.h        # IDs and constants
├── DiscordRPCPlugin.json        # Plugin metadata
└── CMakeLists.txt               # Build configuration
```

## Contributing

Contributions welcome! Areas for improvement:

- Settings UI (enable/disable, custom status)
- More file type icons
- Activity tracking and statistics
- Localization

## License

MIT

## References

- [Qt Creator Plugin Development](https://doc.qt.io/qtcreator-extending/)
- [Discord RPC Documentation](https://discord.com/developers/docs/activities/rpc)
- [discord-rpc C++ Library](https://github.com/discord/discord-rpc)
- [Cute Discord Presence](https://github.com/eduardoc7/qtcreator-plugin-discord-presence) — Original inspiration
