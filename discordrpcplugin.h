#pragma once

#include <extensionsystem/iplugin.h>

namespace DiscordRPC::Internal {

class DiscordRPCPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "DiscordRPCPlugin.json")

public:
    DiscordRPCPlugin();
    ~DiscordRPCPlugin() override;

    Utils::Result<> initialize(const QStringList &arguments) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;

private:
    class DiscordRPCPluginPrivate *d = nullptr;
};

} // namespace DiscordRPC::Internal
