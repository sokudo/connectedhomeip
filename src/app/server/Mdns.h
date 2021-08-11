/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <core/CHIPError.h>
#include <stddef.h>
#include <support/BitFlags.h>

namespace chip {
namespace app {
namespace Mdns {

enum class CommissioningMode
{
    kDisabled,
    kEnabled,
    kEnabledAsAdditionalCommissioning
};

/// Start operational advertising
CHIP_ERROR AdvertiseOperational();

/// Set MDNS commissioner advertisement
CHIP_ERROR AdvertiseCommissioner();

/// Set MDNS commissionable node advertisement
CHIP_ERROR AdvertiseCommissionableNode(CommissioningMode mode);

/// (Re-)starts the minmdns server using default state
// Commissioning mode is enabled if device has not yet been commissioned
void StartServer();

/// (Re-)starts the minmdns server
//
// NOTE: when device has never been commissioned, kAdditionalCommissioning flag will be ignored
//
void StartServer(CommissioningMode mode);

CHIP_ERROR GenerateRotatingDeviceId(char rotatingDeviceIdHexBuffer[], size_t rotatingDeviceIdHexBufferSize);

/// Generates the (random) instance name that a CHIP device is to use for pre-commissioning DNS-SD
CHIP_ERROR GetCommissionableInstanceName(char * buffer, size_t bufferLen);

} // namespace Mdns
} // namespace app
} // namespace chip
