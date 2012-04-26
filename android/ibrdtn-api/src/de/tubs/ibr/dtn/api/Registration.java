package de.tubs.ibr.dtn.api;

import java.io.Serializable;
import java.util.LinkedList;
import java.util.List;

import android.os.Parcel;
import android.os.Parcelable;

public class Registration implements Parcelable, Serializable {
	
	/**
	 * serial id for serializable objects 
	 */
	private static final long serialVersionUID = 8857626607076622551L;
	
	private String endpoint = null;
	private List<GroupEndpoint> groups = new LinkedList<GroupEndpoint>();
	
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
	
	public List<GroupEndpoint> getGroups()
	{
		return this.groups;
	}
	
    public static final Creator<Registration> CREATOR = new Creator<Registration>() {
        @Override
        public Registration createFromParcel(final Parcel source) {
        	Registration r = new Registration(source.readString());
        	source.readTypedList(r.groups, GroupEndpoint.CREATOR );
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
		dest.writeString(endpoint);
		dest.writeTypedList(groups);
	}
}
