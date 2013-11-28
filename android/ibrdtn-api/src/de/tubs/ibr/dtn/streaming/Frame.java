package de.tubs.ibr.dtn.streaming;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import android.os.Parcel;
import android.os.Parcelable;

public class Frame implements Parcelable, Comparable<Frame> {
    public byte[] data;
    public int offset;
    public int number;
    
    public void clear() {
        data = null;
        offset = 0;
        number = 0;
    }
    
    @Override
    public String toString() {
        if (data == null) {
            return "frame<" + offset + "." + number + ">[0]";
        }
        return "frame<" + offset + "." + number + ">[" + data.length + "]";
    }

    @Override
    public int compareTo(Frame another) {
        if (offset > another.offset) {
            return 1;
        }
        else if (offset < another.offset) {
            return -1;
        }
        else if (number > another.number) {
            return 1;
        }
        else if (number < another.number) {
            return -1;
        }
        return 0;
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof Frame) {
            Frame f = (Frame)o;
            return (offset == f.offset) && (number == f.number);
        }
        return super.equals(o);
    }
    
    public void parse(DataInputStream stream) throws IOException {
        int length = stream.readInt();
        
        if (length < 0) {
            throw new IOException("negative frame length decoded");
        }
        
        if (length > 512000) {
            throw new IOException("invalid frame length decoded");
        }

        data = new byte[length];
        if (length > 0) {
            stream.read(data);
        }
    }
    
    public void write(DataOutputStream stream) throws IOException {
        if (data == null) {
            stream.writeInt(0);
        } else {
            stream.writeInt(data.length);
            stream.write(data);
        }
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(offset);
        dest.writeInt(number);
        
        dest.writeInt(data.length);
        dest.writeByteArray(data);
    }
    
    public static final Creator<Frame> CREATOR = new Creator<Frame>() {
        public Frame createFromParcel(final Parcel source) {
            Frame frame = new Frame();
            
            frame.offset = source.readInt();
            frame.number = source.readInt();
            
            int size = source.readInt();
            frame.data = new byte[size];
            source.readByteArray(frame.data);
            
            return frame;
        }

        @Override
        public Frame[] newArray(int size) {
            return new Frame[size];
        }
    };
}
