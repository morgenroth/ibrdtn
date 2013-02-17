#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <jni.h>
#include <android/log.h>

//#include "NativeLibraryWrapper.h"
#include "Configuration.h"
#include <ibrcommon/Logger.h>
#include <ibrcommon/data/File.h>

#include "ibrdtnd.h"

// static JavaVM *gJavaVM;
// const char *kInterfacePath = "de/tubs/ibr/dtn/service/NativeLibraryWrapper";

#define  ANDROID_LOG_TAG "IBR-DTN NativeLibraryWrapper"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, __VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, ANDROID_LOG_TAG, __VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN, ANDROID_LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, ANDROID_LOG_TAG, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL 
Java_de_tubs_ibr_dtn_service_NativeLibraryWrapper_daemonInitialize(JNIEnv * env, jobject obj)
{
	/**
	 * setup logging capabilities
	 */

	// logging options
	unsigned char logopts = ibrcommon::Logger::LOG_DATETIME | ibrcommon::Logger::LOG_LEVEL | ibrcommon::Logger::LOG_TAG;

	// error filter
	const unsigned char logerr = ibrcommon::Logger::LOGGER_ERR | ibrcommon::Logger::LOGGER_CRIT;

	// logging filter, everything but debug, err and crit
	const unsigned char logstd = ibrcommon::Logger::LOGGER_ALL ^ (ibrcommon::Logger::LOGGER_DEBUG | logerr);

	// syslog filter, everything but DEBUG and NOTICE
	const unsigned char logsys = ibrcommon::Logger::LOGGER_ALL ^ (ibrcommon::Logger::LOGGER_DEBUG | ibrcommon::Logger::LOGGER_NOTICE);

	dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();

 	// enable ring-buffer
 	ibrcommon::Logger::enableBuffer(200);
 
// 	// enable timestamps in logging if requested
// 	if (conf.getLogger().display_timestamps())
// 	{
// 		logopts = (~(ibrcommon::Logger::LOG_DATETIME) & logopts) | ibrcommon::Logger::LOG_TIMESTAMP;
// 	}
 
 	// init syslog
 	ibrcommon::Logger::enableAsync(); // enable asynchronous logging feature (thread-safe)
 	ibrcommon::Logger::enableSyslog("ibrdtn-daemon", 0, 0, logsys);
 
// 	if (!conf.getDebug().quiet())
// 	{
// 		// add logging to the cout
// 		ibrcommon::Logger::addStream(std::cout, logstd, logopts);
// 
// 		// add logging to the cerr
// 		ibrcommon::Logger::addStream(std::cerr, logerr, logopts);
// 	}
// 
// 	// activate debugging
// 	if (conf.getDebug().enabled() && !conf.getDebug().quiet())
// 	{
// 		// init logger
// 		ibrcommon::Logger::setVerbosity(conf.getDebug().level());
// 
// 		ibrcommon::Logger::addStream(std::cout, ibrcommon::Logger::LOGGER_DEBUG, logopts);
// 
// 		_debug = true;
// 	}

	// load the configuration file
	conf.load("/data/data/de.tubs.ibr.dtn/files/config");

 	try {
 		const ibrcommon::File &lf = conf.getLogger().getLogfile();
 		ibrcommon::Logger::setLogfile(lf, ibrcommon::Logger::LOGGER_ALL ^ ibrcommon::Logger::LOGGER_DEBUG, logopts);
 	} catch (const dtn::daemon::Configuration::ParameterNotSetException&) { };
 
 	// greeting
 	IBRCOMMON_LOGGER(info) << "IBR-DTN daemon " << conf.version() << IBRCOMMON_LOGGER_ENDL;
 
// 	if (conf.getDebug().enabled())
// 	{
// 		IBRCOMMON_LOGGER(info) << "debug level set to " << conf.getDebug().level() << IBRCOMMON_LOGGER_ENDL;
// 	}

 	try {
 		const ibrcommon::File &lf = conf.getLogger().getLogfile();
 		IBRCOMMON_LOGGER(info) << "use logfile for output: " << lf.getPath() << IBRCOMMON_LOGGER_ENDL;
 	} catch (const dtn::daemon::Configuration::ParameterNotSetException&) { };

	LOGI("Before ibrdtn_daemon_initialize");
 	ibrdtn_daemon_initialize();
	LOGI("After ibrdtn_daemon_initialize");
}


JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeLibraryWrapper_daemonMainLoop(JNIEnv * env, jobject obj)
{
	LOGI("Before ibrdtn_daemon_main_loop");
	ibrdtn_daemon_main_loop();
	LOGI("After ibrdtn_daemon_main_loop");

	// stop the asynchronous logger
 	ibrcommon::Logger::stop();
}


JNIEXPORT void JNICALL
Java_de_tubs_ibr_dtn_service_NativeLibraryWrapper_daemonShutdown(JNIEnv * env, jobject obj)
{
	LOGI("Before ibrdtn_daemon_shutdown");
	ibrdtn_daemon_shutdown();
	LOGI("After ibrdtn_daemon_shutdown");
}

#ifdef __cplusplus
}
#endif
