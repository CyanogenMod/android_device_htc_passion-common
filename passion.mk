#
# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

PRODUCT_PROPERTY_OVERRIDES :=
    ro.media.dec.jpeg.memcap=20000000

# Don't set /proc/sys/vm/dirty_ratio to 0 when USB mounting
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vold.umsdirtyratio=20

DEVICE_PACKAGE_OVERLAYS := device/htc/passion-common/overlay

PRODUCT_COPY_FILES := \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/base/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/base/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/base/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml \
    frameworks/base/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \

# media config xml file
PRODUCT_COPY_FILES += \
    device/htc/passion-common/media_profiles.xml:system/etc/media_profiles.xml

PRODUCT_PACKAGES := \
    sensors.mahimahi \
    lights.mahimahi \
    librs_jni \
    gralloc.qsd8k \
    copybit.qsd8k \
    gps.mahimahi \
    libOmxCore \
    libOmxVidEnc \
    com.android.future.usb.accessory

# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_COPY_FILES += \
    device/htc/passion-common/mahimahi-keypad.kl:system/usr/keylayout/mahimahi-keypad.kl \
    device/htc/passion-common/h2w_headset.kl:system/usr/keylayout/h2w_headset.kl \
    device/htc/passion-common/synaptics-rmi-touchscreen.idc:system/usr/idc/synaptics-rmi-touchscreen.idc \
    device/htc/passion-common/vold.fstab:system/etc/vold.fstab


PRODUCT_COPY_FILES += \
    device/htc/passion-common/bcm4329.ko:system/lib/modules/bcm4329.ko

ifeq ($(TARGET_PREBUILT_KERNEL),)
LOCAL_KERNEL := device/htc/passion-common/kernel
else
LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel

$(call inherit-product-if-exists, vendor/htc/passion-common/passion-vendor.mk)

# media profiles and capabilities spec
$(call inherit-product, device/htc/passion-common/media_a1026.mk)

# stuff common to all HTC phones
$(call inherit-product, device/htc/common/common.mk)

