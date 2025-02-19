/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "LEDWidget.h"

#include "cybsp.h"
#include "cyhal.h"
#include <platform/CHIPDeviceLayer.h>

void LEDWidget::Init(int ledNum)
{
    mLastChangeTimeUS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mLedNum           = ledNum;
    mState            = CYBSP_LED_STATE_OFF;

    cyhal_gpio_init((cyhal_gpio_t) ledNum, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
}

void LEDWidget::Invert(void)
{
    Set(!mState);
}

void LEDWidget::Set(bool state)
{
    mLastChangeTimeUS = mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet(state);
}

void LEDWidget::Blink(uint32_t changeRateMS)
{
    Blink(changeRateMS, changeRateMS);
}

void LEDWidget::Blink(uint32_t onTimeMS, uint32_t offTimeMS)
{
    mBlinkOnTimeMS  = onTimeMS;
    mBlinkOffTimeMS = offTimeMS;
    Animate();
}

void LEDWidget::Animate()
{
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        chip::System::Clock::MonotonicMicroseconds nowUS            = chip::System::SystemClock().GetMonotonicMicroseconds();
        chip::System::Clock::MonotonicMicroseconds stateDurUS       = ((mState) ? mBlinkOnTimeMS : mBlinkOffTimeMS) * 1000LL;
        chip::System::Clock::MonotonicMicroseconds nextChangeTimeUS = mLastChangeTimeUS + stateDurUS;

        if (chip::System::Clock::IsEarlier(nextChangeTimeUS, nowUS))
        {
            DoSet(!mState);
            mLastChangeTimeUS = nowUS;
        }
    }
}

void LEDWidget::DoSet(bool state)
{
    if (mState != state)
    {
        cyhal_gpio_write((cyhal_gpio_t) mLedNum, ((state) ? CYBSP_LED_STATE_ON : CYBSP_LED_STATE_OFF));
    }
    mState = state;
}
