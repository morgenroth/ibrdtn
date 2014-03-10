/*
 * Intent.java
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

public class Intent {
    public static final String CATEGORY_SERVICES = "de.tubs.ibr.dtn.intent.category.SERVICES";
    
	public static final String DTNAPP = "de.tubs.ibr.dtn.intent.DTNAPP";
	
	/**
	 * An intent with action ENDPOINT_INTERACT is called if the user has selected
	 * an endpoint to interact with.
	 * 
	 * This intent should always carry an EXTRA_ENDPOINT
	 */
	public static final String ENDPOINT_INTERACT = "de.tubs.ibr.dtn.intent.ENDPOINT_INTERACT";
	
	public static final String REGISTRATION = "de.tubs.ibr.dtn.intent.REGISTRATION";
	public static final String REGISTER = "de.tubs.ibr.dtn.intent.REGISTER";
	public static final String UNREGISTER = "de.tubs.ibr.dtn.intent.UNREGISTER";
	
	public static final String RECEIVE = "de.tubs.ibr.dtn.intent.RECEIVE";
	public static final String STATUS_REPORT = "de.tubs.ibr.dtn.intent.STATUS_REPORT";
	public static final String CUSTODY_SIGNAL = "de.tubs.ibr.dtn.intent.CUSTODY_SIGNAL";
	
	public static final String STATE = "de.tubs.ibr.dtn.intent.STATE";
	public static final String EVENT = "de.tubs.ibr.dtn.intent.EVENT";
	
	public static final String NEIGHBOR = "de.tubs.ibr.dtn.intent.NEIGHBOR";
	public static final String NEIGHBOR_SELECTED = "de.tubs.ibr.dtn.intent.NEIGHBOR_SELECTED";

    public static final String EXTRA_ENDPOINT = "de.tubs.ibr.dtn.ENDPOINT";
    public static final String EXTRA_NODE = "de.tubs.ibr.dtn.NODE";
    public static final String EXTRA_INTENT = "de.tubs.ibr.dtn.intent.INTENT";
    public static final String EXTRA_PENDING_INTENT = "de.tubs.ibr.dtn.intent.PENDING_INTENT";
}
