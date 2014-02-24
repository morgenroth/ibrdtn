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

import java.util.StringTokenizer;

import android.os.Parcel;
import android.os.Parcelable;
import de.tubs.ibr.dtn.api.Bundle.ProcFlags;

public class BundleID implements Parcelable, Comparable<BundleID> {
	
	private SingletonEndpoint source = null;
	private Timestamp timestamp = null;
	private Long sequencenumber = null;
	
	private boolean fragment = false;
	private Long fragment_offset = 0L;
	private Long fragment_payload = 0L;

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
		this.fragment_payload = b.getFragmentPayload();
	}
	
	public BundleID(SingletonEndpoint source, Timestamp timestamp, Long sequencenumber)
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
    public boolean equals(Object o) {
	    if (o instanceof BundleID) {
	        BundleID foreign_id = (BundleID)o;

	        if (timestamp != null)
	            if (!timestamp.equals(foreign_id.timestamp)) return false;
	        
	        if (sequencenumber != null)
	            if (!sequencenumber.equals(foreign_id.sequencenumber)) return false;
	        
	        if (source != null)
	            if (!source.equals(foreign_id.source)) return false;
	        
	        if (fragment) {
	            if (!foreign_id.fragment) return false;
	            
	            if (fragment_offset != null)
	                if (!fragment_offset.equals(foreign_id.fragment_offset)) return false;
	            
                if (fragment_payload != null)
                    if (!fragment_payload.equals(foreign_id.fragment_payload)) return false;
	        }
	        
	        return true;
	    }
        return super.equals(o);
    }

    @Override
	public String toString() {
		if (fragment)
		{
			return ((this.timestamp == null) ? "null" : String.valueOf(this.timestamp.getValue())) + 
					" " + String.valueOf(this.sequencenumber) + 
					" " + String.valueOf(this.fragment_offset) + 
					" " + String.valueOf(this.fragment_payload) + 
					" " + this.source;
		}
		else
		{
			return ((this.timestamp == null) ? "null" : String.valueOf(this.timestamp.getValue())) + 
					" " + String.valueOf(this.sequencenumber) + 
					" " + this.source;
		}
	}
	
	public static BundleID fromString(String data) {
	    BundleID ret = new BundleID();

	    // split bundle id into tokens
	    StringTokenizer tokenizer = new StringTokenizer( data );
	    
	    // get the number of tokens (fragment or not)
	    int count = tokenizer.countTokens();
	    
	    // read the timestamp
	    ret.timestamp = new Timestamp(Long.valueOf(tokenizer.nextToken()));
	    
	    // read the sequencenumber
	    ret.sequencenumber = Long.valueOf(tokenizer.nextToken());

	    if (count > 3) {
	        // bundle id belongs to a fragment
	        ret.fragment = true;
	        
	        // read the fragment offset
	        ret.fragment_offset = Long.valueOf(tokenizer.nextToken());
	        
            // read the fragment payload
            ret.fragment_payload = Long.valueOf(tokenizer.nextToken());
	    } else {
	        ret.fragment = false;
	        ret.fragment_offset = 0L;
	        ret.fragment_payload = 0L;
	    }
	    
	    // read the source endpoint
	    ret.source = new SingletonEndpoint(tokenizer.nextToken());

	    return ret;
	}

	public Timestamp getTimestamp() {
		return timestamp;
	}

	public void setTimestamp(Timestamp timestamp) {
		this.timestamp = timestamp;
	}

	public Long getSequencenumber() {
		return sequencenumber;
	}

	public void setSequencenumber(Long sequencenumber) {
		this.sequencenumber = sequencenumber;
	}
	
	public boolean isFragment() {
	    return fragment;
	}
	
	public void setFragment(boolean val) {
	    fragment = val;
	}
	
	public Long getFragmentOffset() {
        return fragment_offset;
    }

    public void setFragmentOffset(Long fragment_offset) {
        this.fragment_offset = fragment_offset;
    }

    public Long getFragmentPayload() {
        return fragment_payload;
    }

    public void setFragmentPayload(Long fragment_payload) {
        this.fragment_payload = fragment_payload;
    }

    public int describeContents() {
		return 0;
	}

	public void writeToParcel(Parcel dest, int flags) {
		if (source == null) dest.writeString("");
		else dest.writeString(source.toString());
		
		dest.writeLong( (timestamp == null) ? 0L : timestamp.getValue() );
		dest.writeLong( (sequencenumber == null) ? 0L : sequencenumber );
		
		dest.writeBooleanArray(new boolean[] {fragment});
		dest.writeLong( (fragment_offset == null) ? 0L : fragment_offset );
        dest.writeLong( (fragment_payload == null) ? 0L : fragment_payload );
	}
	
    public static final Creator<BundleID> CREATOR = new Creator<BundleID>() {
        public BundleID createFromParcel(final Parcel source) {
        	BundleID id = new BundleID();
        	
        	String s_eid = source.readString();
        	if (s_eid.length() == 0) id.source = null;
        	else id.source = new SingletonEndpoint(s_eid);
        	
        	Long ts = source.readLong();
        	id.timestamp = (ts == 0) ? null : new Timestamp( ts );
        	id.sequencenumber = source.readLong();
        	
        	boolean[] barray = { false };
        	source.readBooleanArray(barray);
        	id.fragment = barray[0];
        	id.fragment_offset = source.readLong();
        	id.fragment_payload = source.readLong();
        	return id;
        }

        public BundleID[] newArray(final int size) {
            return new BundleID[size];
        }
    };

    @Override
    public int compareTo(BundleID another) {
        if (another == null) return 1;
        
        if (source.compareTo(another.source) > 0) return 1;
        if (source.compareTo(another.source) < 0) return -1;
        
        if (timestamp.compareTo(another.timestamp) > 0) return 1;
        if (timestamp.compareTo(another.timestamp) < 0) return -1;
        
        if (sequencenumber.compareTo(another.sequencenumber) > 0) return 1;
        if (sequencenumber.compareTo(another.sequencenumber) < 0) return -1;
        
        if (fragment) {
            if (!another.fragment) return 1;
            
            if (fragment_offset.compareTo(another.fragment_offset) > 0) return 1;
            if (fragment_offset.compareTo(another.fragment_offset) < 0) return -1;
            
            return fragment_payload.compareTo(another.fragment_payload);
        }
        else {
            if (another.fragment) return -1;
        }
        
        return 0;
    }
}
