/*
 * GroupEndpoint.java
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

import android.os.Parcel;

public class GroupEndpoint implements EID, Serializable, Comparable<GroupEndpoint> {
	
	/**
	 * serial id for serializable objects
	 */
	private static final long serialVersionUID = -5010680868276381487L;
	
	private String _eid = null;
	
	public GroupEndpoint(String id)
	{
		_eid = id;
	}
	
	public String toString()
	{
		return _eid;
	}
	
	public boolean equals(Object o) {
		if (o instanceof GroupEndpoint) {
		    if (_eid != null) {
		        return _eid.equals(((GroupEndpoint)o)._eid);
		    }
		}
		return super.equals(o);
	}

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(final Parcel dest, final int flags) {
        dest.writeString(_eid);
    }

    public static final Creator<GroupEndpoint> CREATOR = new Creator<GroupEndpoint>() {
        public GroupEndpoint createFromParcel(final Parcel source) {
            return new GroupEndpoint(source.readString());
        }

        public GroupEndpoint[] newArray(final int size) {
            return new GroupEndpoint[size];
        }
    };
    
    @Override
    public int hashCode() {
        return _eid.hashCode();
    }
    
    @Override
    public int compareTo(GroupEndpoint another) {
        return _eid.compareTo(another._eid);
    }
}
