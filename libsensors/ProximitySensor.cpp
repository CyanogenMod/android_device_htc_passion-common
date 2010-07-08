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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include <linux/capella_cm3602.h>

#include <cutils/log.h>

#include "ProximitySensor.h"

/*****************************************************************************/

ProximitySensor::ProximitySensor()
    : SensorBase(CM_DEVICE_NAME, "proximity"),
      mHasInitialValue(0),
      mEnabled(0),
      mInputReader(4)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_P;
    mPendingEvent.type = SENSOR_TYPE_PROXIMITY;
    mPendingEvent.reserved0 = 0;
    mPendingEvent.reserved1[0] = 0;
    mPendingEvent.reserved1[1] = 0;
    mPendingEvent.reserved1[2] = 0;
    mPendingEvent.reserved1[3] = 0;

    int flags = 0;
    if (!ioctl(dev_fd, CAPELLA_CM3602_IOCTL_GET_ENABLED, &flags)) {
        if (flags) {
            setInitialState();
        }
    }
}

ProximitySensor::~ProximitySensor() {
}

int ProximitySensor::setInitialState() {
    struct input_absinfo absinfo;
    if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_PROXIMITY), &absinfo)) {
        mPendingEvent.distance = indexToValue(absinfo.value);
        mHasInitialValue = 1;
    }
    return 0;
}

int ProximitySensor::enable(int en) {
    int flags = en ? 1 : 0;
    int err = 0;
    if (flags != mEnabled) {
        err = ioctl(dev_fd, CAPELLA_CM3602_IOCTL_ENABLE, &flags);
        err = err<0 ? -errno : 0;
        LOGE_IF(err, "CAPELLA_CM3602_IOCTL_ENABLE failed (%s)", strerror(-err));
        if (!err) {
            mEnabled = en ? 1 : 0;
            if (en) {
                setInitialState();
            }
        }
    }
    return err;
}

int ProximitySensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    if (mHasInitialValue) {
        struct timespec t;
        t.tv_sec = t.tv_nsec = 0;
        clock_gettime(CLOCK_MONOTONIC, &t);
        mHasInitialValue = 0;
        mPendingEvent.timestamp = int64_t(t.tv_sec)*1000000000LL + t.tv_nsec;
        *data = mPendingEvent;
        return 1;
    }

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {
            if (event->code == EVENT_TYPE_PROXIMITY) {
                mPendingEvent.distance = indexToValue(event->value);
            }
        } else if (type == EV_SYN) {
            mPendingEvent.timestamp = timevalToNano(event->time);
            *data++ = mPendingEvent;
            count--;
            numEventReceived++;
        }
        mInputReader.next();
    }

    return numEventReceived;
}

float ProximitySensor::indexToValue(size_t index) const
{
    return index * PROXIMITY_THRESHOLD_CM;
}
