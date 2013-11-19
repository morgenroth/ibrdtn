package de.tubs.ibr.dtn.streaming;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import de.tubs.ibr.dtn.api.SDNV;

public class StreamHeader {
    public static final int VERSION = 1;
    public int correlator = 0;
    public int offset = 0;
    
    public MediaType media = MediaType.BINARY;
    public BundleType type = BundleType.INITIAL;
    
    public StreamHeader() {
    }
    
    public static StreamHeader parse(DataInputStream stream) throws IOException {
        int version = stream.readChar();
        
        if (version != VERSION)
            throw new IOException("illegal protocol version");
        
        StreamHeader ret = new StreamHeader();
        
        // read bundle type
        ret.type = BundleType.valueOf( stream.readChar() );
        
        // read correlator
        ret.correlator = SDNV.Read(stream);

        switch (ret.type) {
            case INITIAL:
                ret.media = MediaType.valueOf( stream.readChar() );
                break;
            case DATA:
                ret.offset = SDNV.Read(stream);
                break;
            case FIN:
                ret.offset = SDNV.Read(stream);
                break;
            default:
                break;
        }
        
        return ret;
    }
    
    public static void write(DataOutputStream stream, StreamHeader header) throws IOException {
        stream.writeChar(VERSION);
        stream.writeChar(header.type.code);
        SDNV.Write(stream, header.correlator);
        
        switch (header.type) {
            case INITIAL:
                stream.writeChar(header.media.code);
                break;
            case DATA:
                SDNV.Write(stream, header.offset);
                break;
            case FIN:
                SDNV.Write(stream, header.offset);
                break;
            default:
                break;
        }
    }
}
