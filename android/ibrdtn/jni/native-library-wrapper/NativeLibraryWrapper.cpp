#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <jni.h>
#include <android/log.h>

//#include "NativeLibraryWrapper.h"
#include "ibrdtnd.h"

static JavaVM *gJavaVM;
const char *kInterfacePath = "de/tubs/ibr/dtn/service/NativeLibraryWrapper";

#define  LOG_TAG "IBR-DTN NativeLibraryWrapper"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL 
Java_de_tubs_ibr_dtn_service_NativeLibraryWrapper_daemonInitialize(JNIEnv * env, jobject obj)
{
	LOGI("TEST");
 	ibrdtn_daemon_initialize();
	LOGI("TEST after");
}

#ifdef __cplusplus
}
#endif
