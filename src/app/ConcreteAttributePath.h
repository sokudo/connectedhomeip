/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include <app/util/basic-types.h>

namespace chip {
namespace app {

/**
 * A representation of a concrete attribute path.
 */
struct ConcreteAttributePath
{
    ConcreteAttributePath(EndpointId aEndpointId, ClusterId aClusterId, AttributeId aAttributeId) :
        mEndpointId(aEndpointId), mClusterId(aClusterId), mAttributeId(aAttributeId)
    {}

    const EndpointId mEndpointId   = 0;
    const ClusterId mClusterId     = 0;
    const AttributeId mAttributeId = 0;
};
} // namespace app
} // namespace chip
