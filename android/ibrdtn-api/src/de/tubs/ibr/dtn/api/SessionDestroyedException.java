/*
 * SessionDestroyedException.java
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

package de.tubs.ibr.dtn.api;

public class SessionDestroyedException extends Exception {

	/**
	 * serial for this exception
	 */
	private static final long serialVersionUID = -3111946764474786987L;

    public SessionDestroyedException() {}

    public SessionDestroyedException(Exception innerException) {
        this.innerException = innerException;
    }
    
	public SessionDestroyedException(String what) {
		this.what = null;
	}
	
    @Override
	public String toString() {
    	if ((innerException == null) && (what == null)) return super.toString();
    	return "SessionDestroyedException: " + ((innerException == null) ? what : innerException.toString());
	}

	public Exception innerException = null;
    public String what = null;
}
