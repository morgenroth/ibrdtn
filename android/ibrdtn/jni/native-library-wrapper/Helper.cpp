#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "JNIHelp.h"

#define THIS_LOG_TAG "IBR-DTN Native Helper"

bool checkException(JNIEnv* env) {
	if (env->ExceptionCheck() != 0) {
		LOGE("Uncaught exception returned from Java call!");
		env->ExceptionDescribe();
		return true;
	}
	return false;
}

std::string jstringToStdString(JNIEnv* env, jstring jstr) {
	if (!jstr || !env) {
		return std::string();
	}

	const char* s = env->GetStringUTFChars(jstr, 0);
	if (!s) {
		return std::string();
	}
	std::string str(s);
	env->ReleaseStringUTFChars(jstr, s);
	checkException(env);
	return str;
}

jstring stdStringToJstring(JNIEnv* env, std::string str) {
	return env->NewStringUTF(str.c_str());
}
