/*
 * NTService.cpp
 *
 * Copyright (C) 2012 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "config.h"
#include "Configuration.h"
#include <ibrcommon/Logger.h>
#include <ibrcommon/data/File.h>

#include <windows.h>

#include <string.h>
#include <csignal>
#include <set>

#include <sys/types.h>
#include <unistd.h>

#include "ibrdtnd.h"

/**
 * setup logging capabilities
 */

// logging options
unsigned char logopts = ibrcommon::Logger::LOG_DATETIME | ibrcommon::Logger::LOG_LEVEL;

// error filter
const unsigned char logerr = ibrcommon::Logger::LOGGER_ERR | ibrcommon::Logger::LOGGER_CRIT;

// logging filter, everything but debug, err and crit
const unsigned char logstd = ibrcommon::Logger::LOGGER_ALL ^ (ibrcommon::Logger::LOGGER_DEBUG | logerr);

SERVICE_STATUS          ServiceStatus; 
SERVICE_STATUS_HANDLE   hStatus; 
 
int InitService() {
        // create a configuration
        dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();

	// TODO: need to initialize the configuration here

	return 0;

	// return -1 on failure
	// return -1; 
}

void ControlHandler(DWORD request) 
{ 
	switch(request) 
	{ 
	case SERVICE_CONTROL_STOP:
		ibrdtn_daemon_shutdown(); 
		return; 
 
	case SERVICE_CONTROL_SHUTDOWN: 
		ibrdtn_daemon_shutdown():
		return; 
	
	default:
		break;
	} 
 
	// Report current status
	SetServiceStatus (hStatus, &ServiceStatus);
 
	return; 
}

void ServiceMain(int argc, char** argv) {
	int error = 0;
	
	ServiceStatus.dwServiceType = SERVICE_WIN32; 
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING; 
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	hStatus = RegisterServiceCtrlHandler("IBR-DTN", (LPHANDLER_FUNCTION)ControlHandler);

	if (hStatus == (SERVICE_STATUS_HANDLE)0)
	{
		// Registering Control Handler failed
		return;
	}

	// initialize the service
	error = InitService();

	if (error) 
	{
		// Initialization failed; we stop the service
		ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
		ServiceStatus.dwWin32ExitCode = error; 
		SetServiceStatus(hStatus, &ServiceStatus); 

		// exit ServiceMain
		return;
	}

        // create a configuration
        dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();

	// enable ring-buffer
	ibrcommon::Logger::enableBuffer(200);

	ibrcommon::Logger::enableAsync(); // enable asynchronous logging feature (thread-safe)

	// load the configuration file
	conf.load();

	try {
		const ibrcommon::File &lf = conf.getLogger().getLogfile();
		ibrcommon::Logger::setLogfile(lf, ibrcommon::Logger::LOGGER_ALL ^ ibrcommon::Logger::LOGGER_DEBUG, logopts);
	} catch (const dtn::daemon::Configuration::ParameterNotSetException&) { };

	// greeting
	IBRCOMMON_LOGGER(info) << "IBR-DTN daemon " << conf.version() << IBRCOMMON_LOGGER_ENDL;

	try {
		const ibrcommon::File &lf = conf.getLogger().getLogfile();
		IBRCOMMON_LOGGER(info) << "use logfile for output: " << lf.getPath() << IBRCOMMON_LOGGER_ENDL;
	} catch (const dtn::daemon::Configuration::ParameterNotSetException&) { };

	// We report the running status to SCM. 
	ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
	SetServiceStatus (hStatus, &ServiceStatus);

	ibrdtn_daemon_initialize();

	ibrdtn_daemon_main_loop();

	// stop the asynchronous logger
	ibrcommon::Logger::stop();

	// report the stop of the service to SCM
	ServiceStatus.dwWin32ExitCode = 0;  
	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus (hStatus, &ServiceStatus);
}

void main() 
{ 
	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = "IBR-DTN";
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;

	// Start the control dispatcher thread for our service
	StartServiceCtrlDispatcher(ServiceTable);  
}

