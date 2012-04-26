package de.tubs.ibr.dtn.api;

import android.os.Parcel;
import android.os.Parcelable;

public class Block implements Parcelable {
	public Integer type = null;
	public Long procflags = null;
	public Long length = null;
	
	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeInt( (type == null) ? 0 : type );
		dest.writeLong( (procflags == null) ? 0L : procflags );
		dest.writeLong( (length == null) ? 0L : length );
	}
	
    public static final Creator<Block> CREATOR = new Creator<Block>() {
        @Override
        public Block createFromParcel(final Parcel source) {
        	Block b = new Block();
        	b.type = source.readInt();
        	b.procflags = source.readLong();
        	b.length = source.readLong();
        	return b;
        }

        @Override
        public Block[] newArray(final int size) {
            return new Block[size];
        }
    };
}
