package de.tubs.ibr.dtn.streaming;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import de.tubs.ibr.dtn.api.SDNV;

public class Frame implements Comparable<Frame> {
    public byte[] data;
    public int offset;
    public int number;
    
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
    
    public static Frame parse(DataInputStream stream) throws IOException {
        int length = stream.readInt();

        Frame ret = new Frame();
        ret.data = new byte[length];
        if (length > 0) {
            stream.read(ret.data);
        }
        
        return ret;
    }
    
    public static void write(DataOutputStream stream, Frame frame) throws IOException {
        if (frame.data == null) {
            stream.writeInt(0);
        } else {
            stream.writeInt(frame.data.length);
            stream.write(frame.data);
        }
    }
}
