#include <jni.h>
#include "JNIHelp.h"

#define THIS_LOG_TAG "JNI_OnLoad"

/* each class file includes its own register function */
int registerJniHelp(JNIEnv* env);
int register_de_tubs_ibr_dtn_service_NativeDaemonWrapper(JNIEnv *env);

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	LOGI("JNI_OnLoad");
	JNIEnv* env;
	if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
		LOGE("Failed to get the environment using GetEnv()");
		return -1;
	}

	//LOGI("JNI_OnLoad init cached classes in JniConstants:");
	//JniConstants::init(env);

	LOGI("JNI_OnLoad register methods:");
	registerJniHelp(env);
	register_de_tubs_ibr_dtn_service_NativeDaemonWrapper(env);

	LOGI("JNI_OnLoad done");

	return JNI_VERSION_1_6;
}


extern "C" void JNI_OnUnload(JavaVM *vm, void *reserved)
{
	LOGD("JNI_OnUnload");
}
