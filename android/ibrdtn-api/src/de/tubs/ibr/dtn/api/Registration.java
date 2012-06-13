/*
 * Registration.java
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

import java.io.Serializable;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.Set;

import android.os.Parcel;
import android.os.Parcelable;

public class Registration implements Parcelable, Serializable {
	
	/**
	 * serial id for serializable objects 
	 */
	private static final long serialVersionUID = 8857626607076622551L;
	
	private String endpoint = null;
	private Set<GroupEndpoint> groups = new LinkedHashSet<GroupEndpoint>();
	
	public Registration(String endpoint)
	{
		this.endpoint = endpoint;
	}
	
	public void add(GroupEndpoint eid)
	{
		this.groups.add(eid);
	}
	
	public String getEndpoint()
	{
		return this.endpoint;
	}
	
	public Set<GroupEndpoint> getGroups()
	{
		return this.groups;
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof Registration) {
			return this.equals((Registration)o);
		}
		return super.equals(o);
	}

	public boolean equals(Registration other) {
		if (!this.endpoint.equals(other.endpoint)) return false;
		if (this.groups.equals(other.groups)) return false;
		return true;
	}
	
    public static final Creator<Registration> CREATOR = new Creator<Registration>() {
        @Override
        public Registration createFromParcel(final Parcel source) {
        	Registration r = new Registration(source.readString());
        	
        	LinkedList<GroupEndpoint> groups = new LinkedList<GroupEndpoint>();
        	source.readTypedList(groups, GroupEndpoint.CREATOR );
        	r.groups.addAll(groups);
        	
            return r;
        }

        @Override
        public Registration[] newArray(final int size) {
            return new Registration[size];
        }
    };

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeString(this.endpoint);
		LinkedList<GroupEndpoint> groups = new LinkedList<GroupEndpoint>();
		groups.addAll(this.groups);
		dest.writeTypedList(groups);
	}
}
