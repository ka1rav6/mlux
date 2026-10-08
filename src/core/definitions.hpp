/*
 * Contains the definitions of basic `types` used throughout the project`
 * This file is included in almost every other header file.
 *
 * */

#ifndef MLUX_DEFINITIONS_H
#define MLUX_DEFINITIONS_H

#include <cinttypes>

namespace mlux
{

using SessionId = uint64_t;
using WindowId = uint64_t;
using PaneId = uint64_t;
using PluginId = uint64_t;
using CommandId = uint64_t;
using Width = uint16_t;
using Height = uint16_t;
using ProcessId = int32_t;
using FileDescriptor = int32_t;

} // namespace mlux
#endif
