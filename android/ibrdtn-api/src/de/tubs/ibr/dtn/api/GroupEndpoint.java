package de.tubs.ibr.dtn.api;

import java.io.Serializable;

import android.os.Parcel;

public class GroupEndpoint implements EID, Serializable {
	
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
	
    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(final Parcel dest, final int flags) {
        dest.writeString(_eid);
    }

    public static final Creator<GroupEndpoint> CREATOR = new Creator<GroupEndpoint>() {
        @Override
        public GroupEndpoint createFromParcel(final Parcel source) {
            return new GroupEndpoint(source.readString());
        }

        @Override
        public GroupEndpoint[] newArray(final int size) {
            return new GroupEndpoint[size];
        }
    };
}
