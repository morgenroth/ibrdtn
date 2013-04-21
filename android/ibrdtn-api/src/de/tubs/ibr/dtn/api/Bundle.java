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
	private EID destination = null;
	private SingletonEndpoint source = null;
	private SingletonEndpoint custodian = null;
	private SingletonEndpoint reportto = null;
	private Long lifetime = null;
	private Date timestamp = null;
	private Long sequencenumber = null;
	private Long procflags = 0L;
	private Long app_data_length = null;
	private Long fragment_offset = null;
	
	public enum Priority {
		LOW,
		MEDIUM,
		HIGH
	};

	public enum ProcFlags {
		FRAGMENT(1 << 0x00),
		APPDATA_IS_ADMRECORD(1 << 0x01),
		DONT_FRAGMENT(1 << 0x02),
		CUSTODY_REQUESTED(1 << 0x03),
		DESTINATION_IS_SINGLETON(1 << 0x04),
		ACKOFAPP_REQUESTED(1 << 0x05),
		RESERVED_6(1 << 0x06),
		PRIORITY_BIT1(1 << 0x07),
		PRIORITY_BIT2(1 << 0x08),
		CLASSOFSERVICE_9(1 << 0x09),
		CLASSOFSERVICE_10(1 << 0x0A),
		CLASSOFSERVICE_11(1 << 0x0B),
		CLASSOFSERVICE_12(1 << 0x0C),
		CLASSOFSERVICE_13(1 << 0x0D),
		REQUEST_REPORT_OF_BUNDLE_RECEPTION(1 << 0x0E),
		REQUEST_REPORT_OF_CUSTODY_ACCEPTANCE(1 << 0x0F),
		REQUEST_REPORT_OF_BUNDLE_FORWARDING(1 << 0x10),
		REQUEST_REPORT_OF_BUNDLE_DELIVERY(1 << 0x11),
		REQUEST_REPORT_OF_BUNDLE_DELETION(1 << 0x12),
		STATUS_REPORT_REQUEST_19(1 << 0x13),
		STATUS_REPORT_REQUEST_20(1 << 0x14),

		// DTNSEC FLAGS (these are customized flags and not written down in any draft)
		DTNSEC_REQUEST_SIGN(1 << 0x1A),
		DTNSEC_REQUEST_ENCRYPT(1 << 0x1B),
		DTNSEC_STATUS_VERIFIED(1 << 0x1C),
		DTNSEC_STATUS_CONFIDENTIAL(1 << 0x1D),
		DTNSEC_STATUS_AUTHENTICATED(1 << 0x1E),
		IBRDTN_REQUEST_COMPRESSION(1 << 0x1F);
		
		private int value = 0;
		
		private ProcFlags(int i) {
			this.value = i;
		}
		
		public int getValue() {
			return this.value;
		}
	};
	
	public Bundle() {
	}
	
	public Bundle(long procflags) {
		this.procflags = procflags;
	}
	
	public Boolean get(ProcFlags flag) {
		return (flag.getValue() & this.procflags) > 0;
	}
	
	public void set(ProcFlags flag, Boolean value) {
		if (value) {
			this.procflags |= flag.getValue();
		} else {
			this.procflags &= ~(flag.getValue());
		}
	}
	
	public Priority getPriority() {
		if (get(ProcFlags.PRIORITY_BIT1))
		{
			return Priority.MEDIUM;
		}

		if (get(ProcFlags.PRIORITY_BIT2))
		{
			return Priority.HIGH;
		}

		return Priority.LOW;
	}
	
	public void setPriority(Priority p) {
		// set the priority to the real bundle
		switch (p)
		{
		case LOW:
			set(ProcFlags.PRIORITY_BIT1, false);
			set(ProcFlags.PRIORITY_BIT2, false);
			break;

		case HIGH:
			set(ProcFlags.PRIORITY_BIT1, false);
			set(ProcFlags.PRIORITY_BIT2, true);
			break;

		case MEDIUM:
			set(ProcFlags.PRIORITY_BIT1, true);
			set(ProcFlags.PRIORITY_BIT2, false);
			break;
		}
	}
	
	public EID getDestination() {
		return destination;
	}

	public void setDestination(EID destination) {
		this.destination = destination;
		this.set(ProcFlags.DESTINATION_IS_SINGLETON, this.destination instanceof SingletonEndpoint);
	}

	public SingletonEndpoint getSource() {
		return source;
	}

	public void setSource(SingletonEndpoint source) {
		this.source = source;
	}

	public SingletonEndpoint getCustodian() {
		return custodian;
	}

	public void setCustodian(SingletonEndpoint custodian) {
		this.custodian = custodian;
	}

	public SingletonEndpoint getReportto() {
		return reportto;
	}

	public void setReportto(SingletonEndpoint reportto) {
		this.reportto = reportto;
	}

	public Long getLifetime() {
		return lifetime;
	}

	public void setLifetime(Long lifetime) {
		this.lifetime = lifetime;
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

	public Long getAppDataLength() {
		return app_data_length;
	}

	public void setAppDataLength(Long app_data_length) {
		this.app_data_length = app_data_length;
	}

	public Long getFragmentOffset() {
		return fragment_offset;
	}

	public void setFragmentOffset(Long fragment_offset) {
		this.fragment_offset = fragment_offset;
	}

	public Long getProcflags() {
		return procflags;
	}
	
	public int describeContents() {
		return 0;
	}

	public void writeToParcel(Parcel dest, int flags) {
		boolean nullMarker[] = {
			destination != null,
			source != null,
			custodian != null,
			reportto != null,
			lifetime != null,
			timestamp != null,
			sequencenumber != null,
			app_data_length != null,
			fragment_offset != null
		};
		
		// write processing flags
		dest.writeLong( procflags );
		
		// write null marker
		dest.writeBooleanArray(nullMarker);
		
		if (nullMarker[0]) dest.writeString(destination.toString());
		if (nullMarker[1]) dest.writeString(source.toString());
		if (nullMarker[2]) dest.writeString(custodian.toString());
		if (nullMarker[3]) dest.writeString(reportto.toString());
		if (nullMarker[4]) dest.writeLong( lifetime );
		if (nullMarker[5]) dest.writeLong( timestamp.getTime() );
		if (nullMarker[6]) dest.writeLong( sequencenumber );
		if (nullMarker[7]) dest.writeLong( app_data_length );
		if (nullMarker[8]) dest.writeLong( fragment_offset );
	}
	
    public static final Creator<Bundle> CREATOR = new Creator<Bundle>() {
        public Bundle createFromParcel(final Parcel source) {
        	// create bundle
        	Bundle b = new Bundle();
        	
        	// read processing flags
        	b.procflags = source.readLong();
        	
        	// read null marker array
        	boolean nullMarker[] = { false, false, false, false, false, false, false, false, false };
        	source.readBooleanArray(nullMarker);
        	
        	// read destination
        	if (nullMarker[0]) {
	        	if (b.get(ProcFlags.DESTINATION_IS_SINGLETON)) {
	        		b.destination = new SingletonEndpoint(source.readString());
	        	} else {
	        		b.destination = new GroupEndpoint(source.readString());
	        	}
        	} else {
        		b.destination = null;
        	}
        	
        	if (nullMarker[1]) b.source = new SingletonEndpoint(source.readString());
        	else b.source = null;
        	
        	if (nullMarker[2]) b.custodian = new SingletonEndpoint(source.readString());
        	else b.custodian = null;
        	
        	if (nullMarker[3]) b.reportto = new SingletonEndpoint(source.readString());
        	else b.reportto = null;
        	
        	if (nullMarker[4]) b.lifetime = source.readLong();
        	else b.lifetime = null;
        	
        	if (nullMarker[5]) b.timestamp = new Date( source.readLong() );
        	else b.timestamp = null;
        	
        	if (nullMarker[6]) b.sequencenumber = source.readLong();
        	b.sequencenumber = null;
        	
        	if (nullMarker[7]) b.app_data_length = source.readLong();
        	b.app_data_length = null;
        	
        	if (nullMarker[8]) b.fragment_offset = source.readLong();
        	b.fragment_offset = null;
        	
        	return b;
        }

        public Bundle[] newArray(final int size) {
            return new Bundle[size];
        }
    };
}
