/*
 * Event.java
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

import java.util.HashMap;
import java.util.Map;

import android.os.Parcel;
import android.os.Parcelable;

public class Event implements Parcelable {
	
	private String _action = null;
	private String _name = null;
	private Map<String, String> _attributes = null;
	
	public Event(String name, String action, Map<String, String> attrs)
	{
		this._name = name;
		this._action = action;
		this._attributes = attrs;
	}

	public String getAction() {
		return _action;
	}

	public String getName() {
		return _name;
	}

	public Map<String, String> getAttributes() {
		return _attributes;
	}

	public void setAttributes(Map<String, String> attributes) {
		this._attributes = attributes;
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeString(_name);
		dest.writeString(_action);
		dest.writeMap(_attributes);
	}
	
    public static final Creator<Event> CREATOR = new Creator<Event>() {
        @Override
        public Event createFromParcel(final Parcel source) {
        	String name = source.readString();
        	String action = source.readString();
        	HashMap<String, String> attrs = new HashMap<String, String>();      	
        	//evt._attributes = source.read

        	return new Event(name, action, attrs);
        }

        @Override
        public Event[] newArray(final int size) {
            return new Event[size];
        }
    };
}
