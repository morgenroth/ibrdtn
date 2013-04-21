/*
 * Block.java
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

import java.util.Date;

import de.tubs.ibr.dtn.api.Bundle.ProcFlags;

import android.os.Parcel;
import android.os.Parcelable;

public class BundleID implements Parcelable {
	
	private SingletonEndpoint source = null;
	private Date timestamp = null;
	private Long sequencenumber = null;
	
	private Boolean fragment = false;
	private Long fragment_offset = 0L;

	public BundleID()
	{
	}
	
	public BundleID(Bundle b)
	{
		this.source = b.getSource();
		this.timestamp = b.getTimestamp();
		this.sequencenumber = b.getSequencenumber();
		
		this.fragment = b.get(ProcFlags.FRAGMENT);
		this.fragment_offset = b.getFragmentOffset();
	}
	
	public BundleID(SingletonEndpoint source, Date timestamp, Long sequencenumber)
	{
		this.source = source;
		this.timestamp = timestamp;
		this.sequencenumber = sequencenumber;
	}

	public SingletonEndpoint getSource() {
		return source;
	}

	public void setSource(SingletonEndpoint source) {
		this.source = source;
	}

	@Override
	public String toString() {
		if (fragment)
		{
			return ((this.timestamp == null) ? "null" : String.valueOf(this.timestamp.getTime())) + 
					" " + String.valueOf(this.sequencenumber) + 
					" " + String.valueOf(this.fragment_offset) + 
					" " + this.source;
		}
		else
		{
			return ((this.timestamp == null) ? "null" : String.valueOf(this.timestamp.getTime())) + 
					" " + String.valueOf(this.sequencenumber) + 
					" " + this.source;
		}
	}

	public Date getTimestamp() {
		return timestamp;
	}

	public void setTimestamp(Date timestamp) {
		this.timestamp = timestamp;
	}

	public Long getSequencenumber() {
		return sequencenumber;
	}

	public void setSequencenumber(Long sequencenumber) {
		this.sequencenumber = sequencenumber;
	}
	
	public int describeContents() {
		return 0;
	}

	public void writeToParcel(Parcel dest, int flags) {
		if (source == null) dest.writeString("");
		else dest.writeString(source.toString());
		
		dest.writeLong( (timestamp == null) ? 0L : timestamp.getTime() );
		dest.writeLong( (sequencenumber == null) ? 0L : sequencenumber );
		
		dest.writeBooleanArray(new boolean[] {fragment});
		dest.writeLong( (fragment_offset == null) ? 0L : fragment_offset );
	}
	
    public static final Creator<BundleID> CREATOR = new Creator<BundleID>() {
        public BundleID createFromParcel(final Parcel source) {
        	BundleID id = new BundleID();
        	
        	String s_eid = source.readString();
        	if (s_eid.length() == 0) id.source = null;
        	else id.source = new SingletonEndpoint(s_eid);
        	
        	Long ts = source.readLong();
        	id.timestamp = (ts == 0) ? null : new Date( ts );
        	id.sequencenumber = source.readLong();
        	
        	boolean[] barray = { false };
        	source.readBooleanArray(barray);
        	id.fragment = barray[0];
        	id.fragment_offset = source.readLong();
        	return id;
        }

        public BundleID[] newArray(final int size) {
            return new BundleID[size];
        }
    };
}
