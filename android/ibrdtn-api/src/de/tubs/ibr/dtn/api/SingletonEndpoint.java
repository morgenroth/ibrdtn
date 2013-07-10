/*
 * SingletonEndpoint.java
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

public class SingletonEndpoint implements EID, Serializable {
	
	/**
     * serial id for serializable objects
     */
    private static final long serialVersionUID = -8294770576249979527L;
    
    public static final SingletonEndpoint ME = new SingletonEndpoint("api:me");
	private String _eid = null;
	
	public SingletonEndpoint(String id)
	{
		_eid = id;
	}
	
	public String toString()
	{
		return _eid;
	}
	
    @Override
    public boolean equals(Object o) {
        if (o instanceof SingletonEndpoint) {
            if (_eid != null) {
                return _eid.equals(((SingletonEndpoint)o)._eid);
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

    public static final Creator<SingletonEndpoint> CREATOR = new Creator<SingletonEndpoint>() {
        public SingletonEndpoint createFromParcel(final Parcel source) {
            return new SingletonEndpoint(source.readString());
        }

        public SingletonEndpoint[] newArray(final int size) {
            return new SingletonEndpoint[size];
        }
    };
}
