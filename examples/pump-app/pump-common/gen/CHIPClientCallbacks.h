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

// THIS FILE IS GENERATED BY ZAP

#pragma once

#include <app/Command.h>
#include <app/InteractionModelEngine.h>
#include <app/common/gen/af-structs.h>
#include <app/util/af-enums.h>
#include <inttypes.h>
#include <lib/support/FunctionTraits.h>
#include <lib/support/Span.h>

// Note: The IMDefaultResponseCallback is a bridge to the old CallbackMgr before IM is landed, so it still accepts EmberAfStatus
// instead of IM status code.
// #6308 should handle IM error code on the application side, either modify this function or remove this.
bool IMDefaultResponseCallback(const chip::app::Command * commandObj, EmberAfStatus status);
bool IMReadReportAttributesResponseCallback(const chip::app::ReadClient * apReadClient, const chip::app::ClusterInfo & aPath,
                                            chip::TLV::TLVReader * apData, chip::Protocols::InteractionModel::ProtocolCode status);

// Global Response Callbacks
typedef void (*DefaultSuccessCallback)(void * context);
typedef void (*DefaultFailureCallback)(void * context, uint8_t status);
typedef void (*BooleanAttributeCallback)(void * context, bool value);
typedef void (*Int8uAttributeCallback)(void * context, uint8_t value);
typedef void (*Int8sAttributeCallback)(void * context, int8_t value);
typedef void (*Int16uAttributeCallback)(void * context, uint16_t value);
typedef void (*Int16sAttributeCallback)(void * context, int16_t value);
typedef void (*Int32uAttributeCallback)(void * context, uint32_t value);
typedef void (*Int32sAttributeCallback)(void * context, int32_t value);
typedef void (*Int64uAttributeCallback)(void * context, uint64_t value);
typedef void (*Int64sAttributeCallback)(void * context, int64_t value);
typedef void (*StringAttributeCallback)(void * context, const chip::ByteSpan value);
typedef void (*AttributeResponseFilter)(chip::TLV::TLVReader * data, chip::Callback::Cancelable * onSuccess,
                                        chip::Callback::Cancelable * onFailure);

/**
 * BasicAttributeFilter accepts the actual type of onSuccess callback as template parameter.
 * It will check whether the type of the TLV data is expected by onSuccess callback.
 * If a non expected value received, onFailure callback will be called with EMBER_ZCL_STATUS_INVALID_VALUE.
 */
template <typename CallbackType>
void BasicAttributeFilter(chip::TLV::TLVReader * data, chip::Callback::Cancelable * onSuccess,
                          chip::Callback::Cancelable * onFailure)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    typename chip::FunctionTraits<CallbackType>::template ArgType<1> value;

    if ((err = data->Get(value)) == CHIP_NO_ERROR)
    {
        chip::Callback::Callback<CallbackType> * cb = chip::Callback::Callback<CallbackType>::FromCancelable(onSuccess);
        cb->mCall(cb->mContext, value);
    }
    else
    {
        ChipLogError(Zcl, "Failed to get value from TLV data for attribute reading response: %d", err);
        chip::Callback::Callback<DefaultFailureCallback> * cb =
            chip::Callback::Callback<DefaultFailureCallback>::FromCancelable(onFailure);
        cb->mCall(cb->mContext, EMBER_ZCL_STATUS_INVALID_VALUE);
    }
}

template <>
void BasicAttributeFilter<StringAttributeCallback>(chip::TLV::TLVReader * data, chip::Callback::Cancelable * onSuccess,
                                                   chip::Callback::Cancelable * onFailure);

typedef void (*ReadReportingConfigurationReportedCallback)(void * context, uint16_t minInterval, uint16_t maxInterval);
typedef void (*ReadReportingConfigurationReceivedCallback)(void * context, uint16_t timeout);

// Cluster Specific Response Callbacks

// List specific responses
