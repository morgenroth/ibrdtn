#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <string>

#include <iostream>
#include <exception>

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

#define THIS_LOG_TAG "IBR-DTN Native NativeDaemonWrapper"

static void daemonInitialize(JNIEnv *env, jclass thisClass, jstring jConfigPath, jboolean jEnableDebug)
{
	//Get the native string from java string
	std::string configPath = jstringToStdString(env, jConfigPath);
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
	try {
		ibrdtn_daemon_initialize();
	} catch (std::exception& e) {
		LOGE("what %s", e.what());
	}
	LOGI("After ibrdtn_daemon_initialize");
}

static void daemonMainLoop(JNIEnv *env, jclass thisClass)
{
	LOGI("Before ibrdtn_daemon_main_loop");
	ibrdtn_daemon_main_loop();
	LOGI("After ibrdtn_daemon_main_loop");

	// stop the asynchronous logger
	ibrcommon::Logger::stop();
}

static void daemonShutdown(JNIEnv *env, jclass thisClass)
{
	LOGI("Before ibrdtn_daemon_shutdown");
	ibrdtn_daemon_shutdown();
	LOGI("After ibrdtn_daemon_shutdown");
}

static void daemonReload(JNIEnv *env, jclass thisClass)
{
	LOGI("Before ibrdtn_daemon_reload");
	ibrdtn_daemon_reload();
	LOGI("After ibrdtn_daemon_reload");
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

static jobjectArray getNeighbors(JNIEnv *env, jclass thisClass)
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

	return returnArray;
}

static void addConnection(JNIEnv *env, jclass thisClass, jstring jEid, jstring jProtocol, jstring jAddress, jstring jPort)
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

static void removeConnection(JNIEnv *env, jclass thisClass, jstring jEid, jstring jProtocol, jstring jAddress, jstring jPort)
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

static JNINativeMethod sMethods[] = {
		{ "daemonInitialize", "(Ljava/lang/String;Z)V", (void *) daemonInitialize },
		{ "daemonMainLoop", "()V", (void *) daemonMainLoop },
		{ "daemonShutdown", "()V", (void *) daemonShutdown },
		{ "daemonReload", "()V", (void *) daemonReload },
		{ "getNeighbors", "()[Ljava/lang/String;", (void *) getNeighbors },
		{ "addConnection", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", (void *) addConnection },
		{ "removeConnection", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", (void *) removeConnection },
};

int register_de_tubs_ibr_dtn_service_NativeDaemonWrapper(JNIEnv* env) {
    jclass cls;

    cls = env->FindClass("de/tubs/ibr/dtn/service/NativeDaemonWrapper");
    if (cls == NULL) {
        LOGE("Can't find de/tubs/ibr/dtn/service/NativeDaemonWrapper\n");
        return -1;
    }
    return env->RegisterNatives(cls, sMethods, NELEM(sMethods));
}
