/*
 * DTNService.aidl
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
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

package de.tubs.ibr.dtn;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.api.DTNSession;

interface DTNService {
    /**
     * Get the state of the daemon.
     */
	DaemonState getState();
	
	/**
	 * Determine if the daemon is running or not.
	 */
	boolean isRunning();
	
	/**
	 * Returns the available neighbors of the daemon.
	 */
	List<String> getNeighbors();
	
	/**
	 * Deletes all bundles in the storage of the daemon.
	 * (Do not work if the daemon is running.)
	 */
	void clearStorage();
	
	/**
	 * Get a previously created session.
	 * @returns null if the session is not available.
	 */
	DTNSession getSession(String packageName);
}
