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
import de.tubs.ibr.dtn.api.Node;

interface DTNService {
    /**
     * Get the state of the daemon.
     */
	DaemonState getState();
	
	/**
	 * Get the local endpoint
	 */
	String getEndpoint();
	
	/**
	 * Returns the available neighbors of the daemon.
	 */
	List<Node> getNeighbors();
	
	/**
	 * Get a previously created session.
	 * @returns null if the session is not available.
	 */
	DTNSession getSession(String sessionKey);
	
	/**
	 * Get the version of the daemon
	 * @returns A array with the version at position 0 and the build number at position 1
	 */
	String[] getVersion();
}
