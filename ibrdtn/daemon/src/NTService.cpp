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

#include "NativeDaemon.h"

// marker if the shutdown was called
ibrcommon::Conditional _shutdown_cond;
bool _shutdown = false;

// daemon instance
dtn::daemon::NativeDaemon _dtnd;

// key for registry access
const char* _subkey = "Software\\IBR-DTN";

SERVICE_STATUS          ServiceStatus;
SERVICE_STATUS_HANDLE   hStatus;

void ibrdtn_daemon_shutdown() {
	ibrcommon::MutexLock l(_shutdown_cond);
	_shutdown = true;
	_shutdown_cond.signal(true);
}

int InitService() {
	bool error = false;

	// enable ring-buffer
	ibrcommon::Logger::enableBuffer(200);

	// enable asynchronous logging feature (thread-safe)
	ibrcommon::Logger::enableAsync();

	// load the configuration file
	_dtnd.setLogging("DTNEngine", 1);

	// read configuration from registry
	HKEY hKey = 0;
	char buf[255] = {0};
	DWORD dwType = 0;
	DWORD dwBufSize = sizeof(buf);
	DWORD dwValue = 0;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _subkey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		int logLevel = 1;

		dwType = REG_DWORD;
		dwBufSize = sizeof(dwValue);
		if ( RegQueryValueEx(hKey, "loglevel", NULL, &dwType, (LPBYTE)&dwValue, &dwBufSize) == ERROR_SUCCESS )
		{
			logLevel = dwValue;
		}
		else
		{
			error = true;
		}

		dwType = REG_DWORD;
		dwBufSize = sizeof(dwValue);
		if ( RegQueryValueEx(hKey, "debuglevel", NULL, &dwType, (LPBYTE)&dwValue, &dwBufSize) == ERROR_SUCCESS )
		{
			_dtnd.setDebug(dwValue);
		}
		else
		{
			error = true;
		}

		dwType = REG_SZ;
		dwBufSize = sizeof(buf);
		if ( RegQueryValueEx(hKey, "logfile", NULL, &dwType, (BYTE*)buf, &dwBufSize) == ERROR_SUCCESS )
		{
			_dtnd.setLogFile(buf, logLevel);
		}
		else
		{
			error = true;
		}

		dwType = REG_SZ;
		dwBufSize = sizeof(buf);
		if ( RegQueryValueEx(hKey, "configfile", NULL, &dwType, (BYTE*)buf, &dwBufSize) == ERROR_SUCCESS )
		{
			_dtnd.setConfigFile(buf);
		}
		else
		{
			error = true;
		}

		RegCloseKey(hKey);
	}
	else
	{
		// can not open registry
		error = true;
	}

	if (error) return -1;
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
		ibrdtn_daemon_shutdown();
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

	hStatus = RegisterServiceCtrlHandler("DtnStack", (LPHANDLER_FUNCTION)ControlHandler);

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

	// initialize the daemon up to runlevel "Routing Extensions"
	_dtnd.init(dtn::daemon::RUNLEVEL_ROUTING_EXTENSIONS);

	// We report the running status to SCM.
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus (hStatus, &ServiceStatus);

	ibrcommon::MutexLock l(_shutdown_cond);
	while (!_shutdown) _shutdown_cond.wait();

	_dtnd.init(dtn::daemon::RUNLEVEL_ZERO);

	// stop the asynchronous logger
	ibrcommon::Logger::stop();

	// report the stop of the service to SCM
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus (hStatus, &ServiceStatus);
}

int main()
{
	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = LPSTR("IBR-DTN");
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;

	// Start the control dispatcher thread for our service
	StartServiceCtrlDispatcher(ServiceTable);
	return 0;
}

