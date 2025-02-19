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

#include "BoltLockManager.h"

#include "AppConfig.h"
#include "AppTask.h"
#include <FreeRTOS.h>

BoltLockManager BoltLockManager::sLock;

TimerHandle_t sLockTimer;

CHIP_ERROR BoltLockManager::Init()
{
    // Create FreeRTOS sw timer for lock timer.
    sLockTimer = xTimerCreate("lockTmr",        // Just a text name, not used by the RTOS kernel
                              1,                // == default timer period (mS)
                              false,            // no timer reload (==one-shot)
                              (void *) this,    // init timer id = lock obj context
                              TimerEventHandler // timer callback handler
    );

    if (sLockTimer == NULL)
    {
        P6_LOG("sLockTimer timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    mState              = State::kLockingCompleted;
    mAutoLockTimerArmed = false;
    mAutoRelock         = false;
    mAutoLockDuration   = 0;

    return CHIP_NO_ERROR;
}

void BoltLockManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool BoltLockManager::IsActionInProgress()
{
    return (mState == State::kLockingInitiated || mState == State::kUnlockingInitiated);
}

bool BoltLockManager::IsUnlocked()
{
    return (mState == State::kUnlockingCompleted);
}

void BoltLockManager::EnableAutoRelock(bool aOn)
{
    mAutoRelock = aOn;
}

void BoltLockManager::SetAutoLockDuration(uint32_t aDurationInSecs)
{
    mAutoLockDuration = aDurationInSecs;
}

bool BoltLockManager::InitiateAction(int32_t aActor, Action aAction)
{
    bool action_initiated = false;
    State new_state;

    // Initiate Lock/Unlock Action only when the previous one is complete.
    if (mState == State::kLockingCompleted && aAction == Action::kUnlock)
    {
        action_initiated = true;

        new_state = State::kUnlockingInitiated;
    }
    else if (mState == State::kUnlockingCompleted && aAction == Action::kLock)
    {
        action_initiated = true;

        new_state = State::kLockingInitiated;
    }

    if (action_initiated)
    {
        if (mAutoLockTimerArmed && new_state == State::kLockingInitiated)
        {
            // If auto lock timer has been armed and someone initiates locking,
            // cancel the timer and continue as normal.
            mAutoLockTimerArmed = false;

            CancelTimer();
        }

        StartTimer(ACTUATOR_MOVEMENT_PERIOS_MS);

        // Since the timer started successfully, update the state and trigger callback
        mState = new_state;

        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction, aActor);
        }
    }

    return action_initiated;
}

void BoltLockManager::StartTimer(uint32_t aTimeoutMs)
{
    if (xTimerIsTimerActive(sLockTimer))
    {
        P6_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sLockTimer, (aTimeoutMs / portTICK_PERIOD_MS), 100) != pdPASS)
    {
        P6_LOG("sLockTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void BoltLockManager::CancelTimer(void)
{
    if (xTimerStop(sLockTimer, 0) == pdFAIL)
    {
        P6_LOG("Lock timer timer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void BoltLockManager::TimerEventHandler(TimerHandle_t xTimer)
{
    // Get lock obj context from timer id.
    BoltLockManager * lock = static_cast<BoltLockManager *>(pvTimerGetTimerID(xTimer));

    // The timer event handler will be called in the context of the timer task
    // once sLockTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = lock;
    if (lock->mAutoLockTimerArmed)
    {
        event.Handler = AutoReLockTimerEventHandler;
    }
    else
    {
        event.Handler = ActuatorMovementTimerEventHandler;
    }
    GetAppTask().PostEvent(&event);
}

void BoltLockManager::AutoReLockTimerEventHandler(AppEvent * aEvent)
{
    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->TimerEvent.Context);
    int32_t actor          = 0;

    // Make sure auto lock timer is still armed.
    if (!lock->mAutoLockTimerArmed)
    {
        return;
    }

    lock->mAutoLockTimerArmed = false;

    P6_LOG("Auto Re-Lock has been triggered!");

    lock->InitiateAction(actor, Action::kLock);
}

void BoltLockManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    Action actionCompleted = Action::KInvalid;

    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->TimerEvent.Context);

    if (lock->mState == State::kLockingInitiated)
    {
        lock->mState    = State::kLockingCompleted;
        actionCompleted = Action::kLock;
    }
    else if (lock->mState == State::kUnlockingInitiated)
    {
        lock->mState    = State::kUnlockingCompleted;
        actionCompleted = Action::kUnlock;
    }

    if (actionCompleted != Action::KInvalid)
    {
        if (lock->mActionCompleted_CB)
        {
            lock->mActionCompleted_CB(actionCompleted);
        }

        if (lock->mAutoRelock && actionCompleted == Action::kUnlock)
        {
            // Start the timer for auto relock
            lock->StartTimer(lock->mAutoLockDuration * 1000);

            lock->mAutoLockTimerArmed = true;

            P6_LOG("Auto Re-lock enabled. Will be triggered in %lu seconds", lock->mAutoLockDuration);
        }
    }
}
