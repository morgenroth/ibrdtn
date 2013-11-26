package de.tubs.ibr.dtn.streaming;

import android.os.Parcel;
import android.os.Parcelable;
import de.tubs.ibr.dtn.api.SingletonEndpoint;

public class StreamId implements Parcelable {
    public SingletonEndpoint source = null;
    public int correlator = 0;
    
    @Override
    public String toString() {
        return source + "#" + Integer.valueOf(correlator);
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof StreamId) {
            StreamId other = (StreamId)o;
            if (other.correlator == correlator) {
                if ((source == null) && (other.source == null)) return true;
                return source.equals(other.source);
            }
            return false;
        }
        return super.equals(o);
    }

    @Override
    public int hashCode() {
        if (source == null) return correlator;
        return source.hashCode() ^ correlator;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(correlator);
        dest.writeParcelable(source, flags);
    }
    
    public static final Creator<StreamId> CREATOR = new Creator<StreamId>() {
        public StreamId createFromParcel(final Parcel source) {
            StreamId id = new StreamId();
            
            id.correlator = source.readInt();
            id.source = source.readParcelable(SingletonEndpoint.class.getClassLoader());
            
            return id;
        }

        @Override
        public StreamId[] newArray(int size) {
            return new StreamId[size];
        }
    };
}
