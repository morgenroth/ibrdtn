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

import android.os.Parcel;
import android.os.Parcelable;

public class Bundle implements Parcelable {
	public String destination = null;
	public String source = null;
	public String custodian = null;
	public String reportto = null;
	public Long lifetime = null;
	public Date timestamp = null;
	public Long sequencenumber = null;
	public Long procflags = null;
	public Long app_data_length = null;
	public Long fragment_offset = null;
	
	public int describeContents() {
		return 0;
	}

	public void writeToParcel(Parcel dest, int flags) {
		dest.writeString(destination);
		dest.writeString(source);
		dest.writeString(custodian);
		dest.writeString(reportto);
		dest.writeLong( (lifetime == null) ? 0L : lifetime );
		dest.writeLong( (timestamp == null) ? 0L : timestamp.getTime() );
		dest.writeLong( (sequencenumber == null) ? 0L : sequencenumber );
		dest.writeLong( (procflags == null) ? 0L : procflags );
		dest.writeLong( (app_data_length == null) ? 0L : app_data_length );
		dest.writeLong( (fragment_offset == null) ? 0L : fragment_offset );
	}
	
    public static final Creator<Bundle> CREATOR = new Creator<Bundle>() {
        public Bundle createFromParcel(final Parcel source) {
        	Bundle b = new Bundle();
        	b.destination = source.readString();
        	b.source = source.readString();
        	b.custodian = source.readString();
        	b.reportto = source.readString();
        	b.lifetime = source.readLong();
        	Long ts = source.readLong();
        	b.timestamp = (ts == 0) ? null : new Date( ts );
        	b.sequencenumber = source.readLong();
        	b.procflags = source.readLong();
        	b.app_data_length = source.readLong();
        	b.fragment_offset = source.readLong();
        	return b;
        }

        public Bundle[] newArray(final int size) {
            return new Bundle[size];
        }
    };
}
