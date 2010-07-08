/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>

#include <poll.h>
#include <pthread.h>
#include <sys/select.h>

#include <linux/input.h>

#include <cutils/atomic.h>
#include <cutils/log.h>

#include "nusensors.h"
#include "LightSensor.h"
#include "ProximitySensor.h"
#include "AkmSensor.h"

/*****************************************************************************/

struct sensors_poll_context_t {
    struct sensors_poll_device_t device; // must be first

        sensors_poll_context_t();
        ~sensors_poll_context_t();
    int activate(int handle, int enabled);
    int setDelay(int handle, int64_t ns);
    int pollEvents(sensors_event_t* data, int count);

private:
    struct pollfd mPollFds[3];
    LightSensor mLightSensor;
    ProximitySensor mProximitySensor;
    AkmSensor mAkmSensor;
};

/*****************************************************************************/

sensors_poll_context_t::sensors_poll_context_t()
{
    mPollFds[0].fd = mLightSensor.getFd();
    mPollFds[0].events = POLLIN;
    mPollFds[0].revents = 0;

    mPollFds[1].fd = mProximitySensor.getFd();
    mPollFds[1].events = POLLIN;
    mPollFds[1].revents = 0;

    mPollFds[2].fd = mAkmSensor.getFd();
    mPollFds[2].events = POLLIN;
    mPollFds[2].revents = 0;
}

sensors_poll_context_t::~sensors_poll_context_t() {
}

int sensors_poll_context_t::activate(int handle, int enabled) {
    int err = -EINVAL;
    int idx = -1;
    switch (handle) {
        case ID_A:
            err = mAkmSensor.enable(AkmSensor::Accelerometer, enabled);
            idx = 2;
            break;
        case ID_M:
            err = mAkmSensor.enable(AkmSensor::MagneticField, enabled);
            idx = 2;
            break;
        case ID_O:
            err = mAkmSensor.enable(AkmSensor::Orientation, enabled);
            idx = 2;
            break;
        case ID_T:
            err = mAkmSensor.enable(AkmSensor::Temperature, enabled);
            idx = 2;
            break;
        case ID_P:
            err = mProximitySensor.enable(enabled);
            idx = 1;
            break;
        case ID_L:
            err = mLightSensor.enable(enabled);
            idx = 0;
            break;
    }

    if (!err && enabled && idx>=0) {
        // pretend there is an event, so we return "something" asap.
        mPollFds[idx].revents = POLLIN;
    }

    return err;
}

int sensors_poll_context_t::setDelay(int handle, int64_t ns) {
    switch (handle) {
        case ID_A:
        case ID_M:
        case ID_O:
        case ID_T:
            mAkmSensor.setDelay(ns);
            break;
        case ID_P:
            mProximitySensor.setDelay(ns);
            break;
        case ID_L:
            mLightSensor.setDelay(ns);
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

int sensors_poll_context_t::pollEvents(sensors_event_t* data, int count)
{
    int nbEvents = 0;
    int n = 0;

    do {
        // see if we have some leftover from the last poll()
        for (int i=0 ; count && i<3 ; i++) {
            if (mPollFds[i].revents & POLLIN) {
                int nb = 0;
                if (i == 0) {
                    nb = mLightSensor.readEvents(data, count);
                } else if (i == 1) {
                    nb = mProximitySensor.readEvents(data, count);
                } else if (i == 2) {
                    nb = mAkmSensor.readEvents(data, count);
                }

                if (nb < count) {
                    // no more data for this sensor
                    mPollFds[i].revents = 0;
                }
                count -= nb;
                nbEvents += nb;
                data += nb;
            }
        }

        if (count) {
            // we still have some room, so try to see if we can get
            // some events immediately or just wait we we don't have
            // anything to return
            n = poll(mPollFds, 3, nbEvents ? 0 : -1);
            if (n<0) {
                LOGE("poll() failed (%s)", strerror(errno));
                return -errno;
            }
        }
        // if we have events and space, go read them
    } while (n && count);

    return nbEvents;
}

/*****************************************************************************/

static int poll__close(struct hw_device_t *dev)
{
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    if (ctx) {
        delete ctx;
    }
    return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev,
        int handle, int enabled) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->activate(handle, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t *dev,
        int handle, int64_t ns) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->setDelay(handle, ns);
}

static int poll__poll(struct sensors_poll_device_t *dev,
        sensors_event_t* data, int count) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->pollEvents(data, count);
}

/*****************************************************************************/

int init_nusensors(hw_module_t const* module, hw_device_t** device)
{
    int status = -EINVAL;

    sensors_poll_context_t *dev = new sensors_poll_context_t();
    memset(&dev->device, 0, sizeof(sensors_poll_device_t));

    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version  = 0;
    dev->device.common.module   = const_cast<hw_module_t*>(module);
    dev->device.common.close    = poll__close;
    dev->device.activate        = poll__activate;
    dev->device.setDelay        = poll__setDelay;
    dev->device.poll            = poll__poll;

    *device = &dev->device.common;
    status = 0;
    return status;
}
