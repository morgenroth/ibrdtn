package de.tubs.ibr.dtn.api;

import android.os.Parcel;

public class SingletonEndpoint implements EID {
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
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(final Parcel dest, final int flags) {
        dest.writeString(_eid);
    }

    public static final Creator<SingletonEndpoint> CREATOR = new Creator<SingletonEndpoint>() {
        @Override
        public SingletonEndpoint createFromParcel(final Parcel source) {
            return new SingletonEndpoint(source.readString());
        }

        @Override
        public SingletonEndpoint[] newArray(final int size) {
            return new SingletonEndpoint[size];
        }
    };
}
