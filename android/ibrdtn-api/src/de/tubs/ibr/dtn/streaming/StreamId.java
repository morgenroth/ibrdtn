package de.tubs.ibr.dtn.streaming;

import de.tubs.ibr.dtn.api.SingletonEndpoint;

public class StreamId {
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
}
