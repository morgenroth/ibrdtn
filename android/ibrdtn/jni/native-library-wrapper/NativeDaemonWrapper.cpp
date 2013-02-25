#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <jni.h>
#include <android/log.h>

#include "NativeDaemonWrapper.h"
#include "Configuration.h"
#include <ibrcommon/Logger.h>
#include <ibrcommon/data/File.h>

// dtnd core
#include "core/BundleCore.h"
#include "core/GlobalEvent.h"
#include "core/Node.h"

#include "ibrdtnd.h"

#define  ANDROID_LOG_TAG "IBR-DTN NativeLibraryWrapper Native"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, __VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, ANDROID_LOG_TAG, __VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN, ANDROID_LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, ANDROID_LOG_TAG, __VA_ARGS__)

static JavaVM *gJavaVM;
const char *kLogAdapterPath = "de/tubs/ibr/dtn/daemon/LogAdapter";

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Helper methods
 */
bool checkException(JNIEnv* env) {
	if (env->ExceptionCheck() != 0) {
		LOGE("Uncaught exception returned from Java call!");
		env->ExceptionDescribe();
		return true;
	}
	return false;
}

std::string jstringToStdString(JNIEnv* env, jstring jstr) {
	if (!jstr || !env)
		return std::string();

	const char* s = env->GetStringUTFChars(jstr, 0);
	if (!s)
		return std::string();
	std::string str(s);
	env->ReleaseStringUTFChars(jstr, s);
	checkException(env);
	return str;
}

jstring stdStringToJstring(JNIEnv* env, std::string str) {
	return env->NewStringUTF(str.c_str());
}

/**
 * Helper to init static global objects
 */
/*void initClassHelper(JNIEnv *env, const char *path, jobject *objptr) {
 jclass cls = env->FindClass(path);
 if (!cls) {
 LOGE("initClassHelper: failed to get %s class reference", path);
 return;
 }
 jmethodID constr = env->GetMethodID(cls, "<init>", "()V");
 if (!constr) {
 LOGE("initClassHelper: failed to get %s constructor", path);
 return;
 }
 jobject obj = env->NewObject(cls, constr);
 if (!obj) {
 LOGE("initClassHelper: failed to create a %s object", path);
 return;
 }
 (*objptr) = env->NewGlobalRef(obj);

 }*/

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
	LOGD("JNI_OnLoad");

	JNIEnv *env;
	gJavaVM = vm;

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("Failed to get the environment using GetEnv()");
		return -1;
	}

	// init static objects
	//initClassHelper(env, kLogAdapterPath, &gLogAdapterObject);

	return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
	LOGD("JNI_OnLoad");
}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_daemonInitialize(JNIEnv *env, jclass thisClass, jstring jConfigPath, jboolean jEnableDebug)
{
	//Get the native string from java string
	const char *configPath = env->GetStringUTFChars(jConfigPath, 0);
	bool enableDebug = (bool) jEnableDebug;

	/**
	 * setup logging capabilities
	 */

	// logging options
	unsigned char logopts = ibrcommon::Logger::LOG_DATETIME | ibrcommon::Logger::LOG_LEVEL | ibrcommon::Logger::LOG_TAG;
	unsigned char logsys;

	// activate debugging
	if (enableDebug)
	{
		// init logger
		ibrcommon::Logger::setVerbosity(99);

		// logcat filter, everything
		logsys = ibrcommon::Logger::LOGGER_ALL;
	} else {
		// logcat filter, everything but DEBUG and NOTICE
		logsys = ibrcommon::Logger::LOGGER_ALL ^ (ibrcommon::Logger::LOGGER_DEBUG | ibrcommon::Logger::LOGGER_NOTICE);
	}

	// enable ring-buffer
	ibrcommon::Logger::enableBuffer(200);

	// enable logging to Android's logcat
	ibrcommon::Logger::enableAsync();// enable asynchronous logging feature (thread-safe)
	ibrcommon::Logger::setDefaultTag("IBR-DTN Core");
	ibrcommon::Logger::enableSyslog("ibrdtn-daemon", 0, 0, logsys);

	// load the configuration file
	dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();
	conf.load(configPath);

	try {
		const ibrcommon::File &lf = conf.getLogger().getLogfile();
		ibrcommon::Logger::setLogfile(lf, ibrcommon::Logger::LOGGER_ALL ^ ibrcommon::Logger::LOGGER_DEBUG, logopts);
	} catch (const dtn::daemon::Configuration::ParameterNotSetException&) {};

	// greeting
	IBRCOMMON_LOGGER(info) << "IBR-DTN daemon " << conf.version() << IBRCOMMON_LOGGER_ENDL;

	try {
		const ibrcommon::File &lf = conf.getLogger().getLogfile();
		IBRCOMMON_LOGGER(info) << "use logfile for output: " << lf.getPath() << IBRCOMMON_LOGGER_ENDL;
	} catch (const dtn::daemon::Configuration::ParameterNotSetException&) {};

	LOGI("Before ibrdtn_daemon_initialize");
	ibrdtn_daemon_initialize();
	LOGI("After ibrdtn_daemon_initialize");

	// free memory
	env->ReleaseStringUTFChars(jConfigPath, configPath);
}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_daemonMainLoop(JNIEnv *env, jclass thisClass)
{
	LOGI("Before ibrdtn_daemon_main_loop");
	ibrdtn_daemon_main_loop();
	LOGI("After ibrdtn_daemon_main_loop");

	// stop the asynchronous logger
	ibrcommon::Logger::stop();
}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_daemonShutdown(JNIEnv *env, jclass thisClass)
{
	LOGI("Before ibrdtn_daemon_shutdown");
	ibrdtn_daemon_shutdown();
	LOGI("After ibrdtn_daemon_shutdown");
}

/*static void loggingCallback(const char *s) {
 int status;
 JNIEnv *env;
 bool isAttached = false;

 status = gJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
 if (status < 0) {
 LOGE("callback_handler: failed to get JNI environment, "
 "assuming native thread");
 status = gJavaVM->AttachCurrentThread(&env, NULL);
 if (status < 0) {
 LOGE("callback_handler: failed to attach "
 "current thread");
 return;
 }
 isAttached = true;
 }

 // Construct a Java string
 jstring js = env->NewStringUTF(s);
 jclass interfaceClass = env->FindClass(kLogAdapterPath);
 if (!interfaceClass) {
 LOGE("callback_handler: failed to get class reference");
 if (isAttached)
 gJavaVM->DetachCurrentThread();
 return;
 }
 // Find the callBack method ID
 jmethodID method = env->GetStaticMethodID(interfaceClass, "callback",
 "(Ljava/lang/String;)V");
 if (!method) {
 LOGE("callback_handler: failed to get method ID");
 if (isAttached)
 gJavaVM->DetachCurrentThread();
 return;
 }
 env->CallStaticVoidMethod(interfaceClass, method, js);

 if (isAttached)
 gJavaVM->DetachCurrentThread();
 }

 JNIEXPORT void JNICALL
 Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_toggleLoggingCallback(JNIEnv *env, jclass thisClass, jboolean jEnable)
 {
 bool enable = (bool) jEnable;

 LOGI("activateLoggingCallback");

 loggingCallback("test");
 }*/

JNIEXPORT jobjectArray JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_getNeighbors(JNIEnv *env, jclass thisClass)
{
	// get neighbors
	const std::set<dtn::core::Node> nlist = dtn::core::BundleCore::getInstance().getConnectionManager().getNeighbors();

	// convert to string array
	std::vector<dtn::core::Node> vectorList(nlist.begin(), nlist.end());
	jobjectArray returnArray = (jobjectArray) env->NewObjectArray(vectorList.size(), env->FindClass("java/lang/String"), env->NewStringUTF(""));
	int i;
	for(i = 0; i < vectorList.size(); i++) {
		jstring neighbor = stdStringToJstring(env, vectorList[i].getEID().getString());
		env->SetObjectArrayElement(returnArray, i, neighbor);
	}

	return(returnArray);
}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_addConnection(JNIEnv *env, jclass thisClass, jstring jEid, jstring jProtocol, jstring jAddress, jstring jPort)
{
	// convert java strings to std::string
	std::string eid = jstringToStdString(env, jEid);
	std::string protocol = jstringToStdString(env, jProtocol);
	std::string address = jstringToStdString(env, jAddress);
	std::string port = jstringToStdString(env, jPort);

	dtn::core::Node n(eid);
	dtn::core::Node::Type t = dtn::core::Node::NODE_STATIC_GLOBAL;

	t = dtn::core::Node::NODE_STATIC_LOCAL;

	if (protocol == "tcp")
	{
		std::string uri = "ip=" + address + ";port=" + port + ";";
		n.add(dtn::core::Node::URI(t, dtn::core::Node::CONN_TCPIP, uri, 0, 10));
		dtn::core::BundleCore::getInstance().getConnectionManager().add(n);
	}
	else if (protocol == "udp")
	{
		std::string uri = "ip=" + address + ";port=" + port + ";";
		n.add(dtn::core::Node::URI(t, dtn::core::Node::CONN_UDPIP, uri, 0, 10));
		dtn::core::BundleCore::getInstance().getConnectionManager().add(n);
	}
	else if (protocol == "file")
	{
		n.add(dtn::core::Node::URI(dtn::core::Node::NODE_STATIC_LOCAL, dtn::core::Node::CONN_FILE, address, 0, 10));
		dtn::core::BundleCore::getInstance().getConnectionManager().add(n);
	}

	LOGI("CONNECTION ADDED");
}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_removeConnection(JNIEnv *env, jclass thisClass, jstring jEid, jstring jProtocol, jstring jAddress, jstring jPort)
{
	// convert java strings to std::string
	std::string eid = jstringToStdString(env, jEid);
	std::string protocol = jstringToStdString(env, jProtocol);
	std::string address = jstringToStdString(env, jAddress);
	std::string port = jstringToStdString(env, jPort);

	dtn::core::Node n(eid);
	dtn::core::Node::Type t = dtn::core::Node::NODE_STATIC_GLOBAL;

	t = dtn::core::Node::NODE_STATIC_LOCAL;

	if (protocol == "tcp")
	{
		std::string uri = "ip=" + address + ";port=" + port + ";";
		n.add(dtn::core::Node::URI(t, dtn::core::Node::CONN_TCPIP, uri, 0, 10));
		dtn::core::BundleCore::getInstance().getConnectionManager().remove(n);
	}
	else if (protocol == "udp")
	{
		std::string uri = "ip=" + address + ";port=" + port + ";";
		n.add(dtn::core::Node::URI(t, dtn::core::Node::CONN_UDPIP, uri, 0, 10));
		dtn::core::BundleCore::getInstance().getConnectionManager().remove(n);
	}
	else if (protocol == "file")
	{
		n.add(dtn::core::Node::URI(dtn::core::Node::NODE_STATIC_LOCAL, dtn::core::Node::CONN_FILE, address, 0, 10));
		dtn::core::BundleCore::getInstance().getConnectionManager().remove(n);
	}

	LOGI("CONNECTION REMOVED");
}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_setEndpoint(JNIEnv *env, jclass thisClass, jstring jId) {

}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_addRegistration(JNIEnv *env, jclass thisClass, jstring jEid) {

}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_loadBundle(JNIEnv *env, jclass thisClass, jstring jId) {

}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_getBundle(JNIEnv *env, jclass thisClass) {

}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_loadAndGetBundle(JNIEnv *env, jclass thisClass) {

}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_markDelivered(JNIEnv *env, jclass thisClass, jstring jId) {

}

JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeDaemonWrapper_send(JNIEnv *env, jclass thisClass, jbyteArray jOutput) {

}

#ifdef __cplusplus
}
#endif
