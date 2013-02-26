#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <string>

// local helper
#include "JNIHelp.h"
#include "Helper.h"

// ibrcommon
#include <ibrcommon/Logger.h>
#include <ibrcommon/data/File.h>

// dtnd
#include "ibrdtnd.h"
#include "Configuration.h"
#include "core/BundleCore.h"
#include "core/GlobalEvent.h"
#include "core/Node.h"

#define THIS_LOG_TAG "IBR-DTN Native ClientSession"

static void setEndpoint(JNIEnv *env, jclass thisClass, jstring jId) {
	/*// convert java strings to std::string
	std::string id = jstringToStdString(env, jId);

	dtn::data::EID new_endpoint = dtn::core::BundleCore::local + "/" + id;

	// error checking
	if (new_endpoint == dtn::data::EID())
	{
		LOGE("INVALID ENDPOINT")
	}
	else
	{
		// unsubscribe from the old endpoint and subscribe to the new one
		Registration& reg = _client.getRegistration();
		reg.unsubscribe(_endpoint);
		reg.subscribe(new_endpoint);
		_endpoint = new_endpoint;
		LOGD("OK");
	}*/
}

static void addRegistration(JNIEnv *env, jclass thisClass, jstring jEid) {
	/*// convert java strings to std::string
	std::string id = jstringToStdString(env, jId);

	dtn::data::EID endpoint(id);

	// error checking
	if (endpoint == dtn::data::EID())
	{
		LOGE("INVALID EID");
	}
	else
	{
		_client.getRegistration().subscribe(endpoint);
		LOGD("OK");
	}*/
}

static void loadBundle(JNIEnv *env, jclass thisClass, jstring jId) {

}

static void getBundle(JNIEnv *env, jclass thisClass) {

}

static void loadAndGetBundle(JNIEnv *env, jclass thisClass) {

}

static void markDelivered(JNIEnv *env, jclass thisClass, jstring jId) {

}

static void sendBundle(JNIEnv *env, jclass thisClass, jbyteArray jOutput) {

}

static JNINativeMethod sMethods[] = {
		{ "setEndpoint", "(Ljava/lang/String;)V", (void *) setEndpoint },
		{ "addRegistration", "(Ljava/lang/String;)V", (void *) addRegistration },
		{ "loadBundle", "(Ljava/lang/String;)V", (void *) loadBundle },
		{ "getBundle", "()V", (void *) getBundle },
		{ "loadAndGetBundle", "()V", (void *) loadAndGetBundle },
		{ "markDelivered", "(Ljava/lang/String;)V", (void *) markDelivered },
		{ "sendBundle", "([B)V", (void *) send },
};

int register_de_tubs_ibr_dtn_service_ClientSession(JNIEnv* env) {
    jclass cls;

    cls = env->FindClass("de/tubs/ibr/dtn/service/ClientSession");
    if (cls == NULL) {
        LOGE("Can't find de/tubs/ibr/dtn/service/ClientSession\n");
        return -1;
    }
    return env->RegisterNatives(cls, sMethods, NELEM(sMethods));
}

