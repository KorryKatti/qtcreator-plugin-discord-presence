cmake_minimum_required(VERSION 3.21)

project(DiscordRPCPlugin
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Discord Rich Presence plugin for Qt Creator 19.0.2+"
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QTC_ROOT "/usr/include/qtcreator" CACHE PATH "Qt Creator source root")
set(QTC_LIB_DIR "/usr/lib/qtcreator" CACHE PATH "Qt Creator library directory")
set(DISCORD_RPC_DIR "" CACHE PATH "discord-rpc install prefix")

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_library(${PROJECT_NAME} SHARED
    discordrpcplugin.h
    discordrpcplugin.cpp
    discordrpcmanager.h
    discordrpcmanager.cpp
    discordrpcconstants.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${QTC_ROOT}/src/libs
    ${QTC_ROOT}/src/libs/extensionsystem
    ${QTC_ROOT}/src/libs/utils
    ${QTC_ROOT}/src/libs/QtTaskTree
    ${QTC_ROOT}/src/plugins
    ${QTC_ROOT}/src/plugins/coreplugin
    ${QTC_ROOT}/src/plugins/coreplugin/editormanager
    ${QTC_ROOT}/src/plugins/projectexplorer
    ${DISCORD_RPC_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${QTC_LIB_DIR}/libExtensionSystem.so.19
    ${QTC_LIB_DIR}/libUtils.so.19
    ${QTC_LIB_DIR}/plugins/libCore.so
    ${QTC_LIB_DIR}/plugins/libProjectExplorer.so
    ${DISCORD_RPC_DIR}/lib/libdiscord-rpc.a
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)
