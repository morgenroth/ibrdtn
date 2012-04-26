package de.tubs.ibr.dtn;

import android.os.Parcel;
import android.os.Parcelable;

public enum DaemonState implements Parcelable {
	UNKOWN,
	ONLINE,
	OFFLINE,
	SUSPENDED,
	ERROR;

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(final Parcel dest, final int flags) {
        dest.writeInt(ordinal());
    }

    public static final Creator<DaemonState> CREATOR = new Creator<DaemonState>() {
        @Override
        public DaemonState createFromParcel(final Parcel source) {
            return DaemonState.values()[source.readInt()];
        }

        @Override
        public DaemonState[] newArray(final int size) {
            return new DaemonState[size];
        }
    };
}
