#include "discordrpcplugin.h"
#include "discordrpcmanager.h"

namespace DiscordRPC::Internal {

class DiscordRPCPluginPrivate
{
public:
    DiscordRPCManager *manager = nullptr;
};

DiscordRPCPlugin::DiscordRPCPlugin()
    : d(new DiscordRPCPluginPrivate)
{
}

DiscordRPCPlugin::~DiscordRPCPlugin()
{
    delete d;
}

Utils::Result<> DiscordRPCPlugin::initialize(const QStringList &arguments)
{
    Q_UNUSED(arguments)

    d->manager = new DiscordRPCManager(this);
    return {};
}

void DiscordRPCPlugin::extensionsInitialized()
{
}

ExtensionSystem::IPlugin::ShutdownFlag DiscordRPCPlugin::aboutToShutdown()
{
    return SynchronousShutdown;
}

} // namespace DiscordRPC::Internal
