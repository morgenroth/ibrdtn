package de.tubs.ibr.dtn.api;

import android.os.Parcel;
import android.os.Parcelable;

public enum CallbackMode implements Parcelable {
	SIMPLE,
	FILEDESCRIPTOR,
	PASSTHROUGH;

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(final Parcel dest, final int flags) {
        dest.writeInt(ordinal());
    }

    public static final Creator<CallbackMode> CREATOR = new Creator<CallbackMode>() {
        @Override
        public CallbackMode createFromParcel(final Parcel source) {
            return CallbackMode.values()[source.readInt()];
        }

        @Override
        public CallbackMode[] newArray(final int size) {
            return new CallbackMode[size];
        }
    };
}
