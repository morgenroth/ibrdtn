/*
 * BundleID.java
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
package ibrdtn.api.object;

public class BundleID {
	
	private String source = null;
	private Long timestamp = null;
	private Long sequencenumber = null;

	public BundleID()
	{
	}
	
	public BundleID(String source, Long timestamp, Long sequencenumber)
	{
		this.source = source;
		this.timestamp = timestamp;
		this.sequencenumber = sequencenumber;
	}

	public String getSource() {
		return source;
	}

	public void setSource(String source) {
		this.source = source;
	}

	@Override
	public String toString() {
		return String.valueOf(this.timestamp) + " " + String.valueOf(this.sequencenumber) + " " + this.source;
	}

	public Long getTimestamp() {
		return timestamp;
	}

	public void setTimestamp(Long timestamp) {
		this.timestamp = timestamp;
	}

	public Long getSequencenumber() {
		return sequencenumber;
	}

	public void setSequencenumber(Long sequencenumber) {
		this.sequencenumber = sequencenumber;
	}
}
