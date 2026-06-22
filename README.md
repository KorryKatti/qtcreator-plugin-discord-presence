# Discord RPC Plugin for Qt Creator 19.0.2

A Qt Creator plugin that adds Discord Rich Presence support, showing what you're coding in real-time on your Discord profile.

## Features

✨ **Live Presence** — Shows current project and file in Discord
🎨 **File Type Icons** — Different icons for C++, Python, QML, JavaScript, etc.
⏱️ **Session Timer** — Tracks how long you've been coding
🔄 **Auto-Update** — Updates every 15 seconds as you work
🚫 **Lightweight** — Minimal performance impact

## What It Shows

Your Discord profile will display:
- **Project Name** — Which Qt project you're working on
- **Current File** — The file you're editing
- **File Type** — Language indicator (C++, Python, etc.)
- **Session Duration** — How long you've been coding
- **Qt Creator Icon** — Visual indicator you're using Qt Creator

Example:
```
Editing: main.cpp
In Project: MyApp
with Qt Creator 19.0.2
```

## Requirements

- **Qt Creator 19.0.2** with development files
- **Qt 6.11.1** (or compatible Qt 6.x)
- **discord-rpc** C++ library
- **CMake 3.21+**
- **C++17 compatible compiler**

### Supported Platforms

- ✅ Windows (MSVC, MinGW)
- ✅ Linux (GCC, Clang)
- ✅ macOS (Clang)

## Quick Start

### 1. Clone and Setup

```bash
git clone <your-repo-url> DiscordRPCPlugin
cd DiscordRPCPlugin
```

### 2. Follow Platform-Specific Guide

- **Windows**: See [BUILD_GUIDE_WINDOWS.md](BUILD_GUIDE_WINDOWS.md)
- **Linux**: See [BUILD_GUIDE_LINUX.md](BUILD_GUIDE_LINUX.md)
- **General**: See [DISCORD_RPC_PLUGIN_GUIDE.md](DISCORD_RPC_PLUGIN_GUIDE.md)

### 3. Build and Install

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
cmake --install .  # or copy manually (see guides)
```

### 4. Restart Qt Creator

The plugin will load automatically. Verify in **Help → About Plugins** and search for "DiscordRPC".

## Directory Structure

```
DiscordRPCPlugin/
├── src/
│   ├── discordrpcplugin.h          # Main plugin class
│   ├── discordrpcplugin.cpp
│   ├── discordrpcmanager.h         # Discord RPC management
│   ├── discordrpcmanager.cpp
│   ├── discordrpcconstants.h
│   └── DiscordRPCPlugin.json       # Plugin metadata
├── CMakeLists.txt                  # Build configuration
├── DISCORD_RPC_PLUGIN_GUIDE.md     # Technical guide
├── BUILD_GUIDE_WINDOWS.md          # Windows build instructions
├── BUILD_GUIDE_LINUX.md            # Linux build instructions
└── README.md                        # This file
```

## Configuration

### Discord Application Setup

1. Create an application at [Discord Developer Portal](https://discord.com/developers/applications)
2. Copy your **Client ID**
3. Edit `src/discordrpcmanager.h`:

```cpp
static constexpr const char *DISCORD_CLIENT_ID = "YOUR_CLIENT_ID_HERE";
```

4. Rebuild the plugin

### Customizing Presence

To change what information is displayed, edit `discordrpcmanager.cpp`:

```cpp
void DiscordRPCManager::updatePresence()
{
    // Modify presence.state, presence.details, etc.
    presence.state = "Custom Status";
    presence.details = "Custom Details";
    Discord_UpdatePresence(&presence);
}
```

## Troubleshooting

### Plugin doesn't load

1. Check plugin directory exists: `~/.local/share/QtProject/QtCreator/plugins/19.0.2/`
2. Verify `.so`/`.dll` file is there
3. Check Help → About Plugins for error messages
4. Run: `qtcreator -logging.rules=qtc.extensionsystem=debug 2>&1 | grep discord`

### Discord doesn't update

1. Ensure Discord desktop app is running
2. Verify Client ID is correct in source code
3. Rebuild and reinstall the plugin
4. Check firewall isn't blocking discord-rpc

### Compilation errors

1. Verify all dependencies installed (see BUILD_GUIDE for your platform)
2. Check CMake found all required libraries
3. Ensure discord-rpc is built and installed
4. Verify Qt Creator development files are installed

See the platform-specific build guides for detailed troubleshooting.

## Building from Source

### Quick Build (Linux/macOS)

```bash
# Install dependencies
# Ubuntu: sudo apt install build-essential cmake qt6-base-dev
# macOS: brew install cmake qt

# Clone and build
git clone <repo> && cd DiscordRPCPlugin
mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DDISCORD_RPC_DIR=$HOME/.local/opt/discord-rpc
cmake --build . -j$(nproc)

# Install
mkdir -p ~/.local/share/QtProject/QtCreator/plugins/19.0.2
cp DiscordRPCPlugin.so ~/.local/share/QtProject/QtCreator/plugins/19.0.2/
```

### Windows Build

See [BUILD_GUIDE_WINDOWS.md](BUILD_GUIDE_WINDOWS.md) for detailed steps.

## Development

### Adding Support for New File Types

Edit `src/discordrpcmanager.cpp`:

```cpp
QString DiscordRPCManager::getFileIcon(const QString &filePath)
{
    static const QMap<QString, QString> iconMap{
        {"cpp", "cpp"},      // Add your file types here
        {"rust", "rust"},
        {"go", "go"},
    };
    return iconMap.value(suffix, "file");
}
```

### Adding Settings/Options

1. Create a settings dialog class
2. Register with Qt Creator's settings system
3. Read/write user preferences
4. Apply settings in `discordrpcmanager.cpp`

## Contributing

Contributions welcome! Areas for improvement:

- [ ] Settings UI (enable/disable, custom status)
- [ ] More file type icons
- [ ] Activity tracking and statistics
- [ ] Support for VS Code, Vim extensions
- [ ] Localization
- [ ] Dark/light theme support

## License

MIT License — See LICENSE file for details.

## References

- [Qt Creator Plugin Development](https://doc.qt.io/qtcreator-extending/)
- [Discord RPC Documentation](https://discord.com/developers/docs/activities/rpc)
- [discord-rpc C++ Library](https://github.com/discordapp/discord-rpc)

## Support

For issues or questions:

1. Check the relevant BUILD_GUIDE for your platform
2. Review [DISCORD_RPC_PLUGIN_GUIDE.md](DISCORD_RPC_PLUGIN_GUIDE.md)
3. Check Qt Creator plugin debug output
4. Search existing GitHub issues

## Acknowledgments

Built for Qt Creator 19.0.2 with Qt 6.11.1, following best practices from:
- Official Qt Creator plugin examples
- Existing QtcDRP and BetterQtcDRP plugins (now updated for modern versions)

---

**Happy coding!** 🚀 Now your Discord friends will know you're busy in Qt Creator.
