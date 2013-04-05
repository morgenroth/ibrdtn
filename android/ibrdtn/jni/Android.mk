LOCAL_PATH := $(call my-dir)
JNI_PATH := $(LOCAL_PATH)

# include openssl
include $(JNI_PATH)/openssl_Android.mk

#NDK_PROJECT_PATH := $(JNI_PATH)/openssl
#include $(JNI_PATH)/openssl/jni/Android.mk
#NDK_PROJECT_PATH := $(JNI_PATH)

# include nl-3
include $(JNI_PATH)/nl-3/android_toolchain/jni/Android_static.mk

# include ibr dtn modules
include $(JNI_PATH)/ibrcommon/Android.mk
include $(JNI_PATH)/ibrdtn/Android.mk
include $(JNI_PATH)/dtnd/Android.mk

# include android glue code
include $(JNI_PATH)/android-glue/Android.mk
