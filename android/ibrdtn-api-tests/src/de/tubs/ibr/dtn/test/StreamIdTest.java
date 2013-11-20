package de.tubs.ibr.dtn.test;

import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.streaming.StreamId;
import junit.framework.TestCase;

public class StreamIdTest extends TestCase {

    public void testHashCode() {
        StreamId id1 = new StreamId();
        id1.correlator = 123;
        id1.source = new SingletonEndpoint("dtn://testing");
        
        StreamId id2 = new StreamId();
        id2.correlator = 123;
        id2.source = new SingletonEndpoint("dtn://testing");
        
        assertEquals(id1.hashCode(), id2.hashCode());
    }

    public void testToString() {
        StreamId id = new StreamId();
        id.correlator = 123;
        id.source = new SingletonEndpoint("dtn://testing");
        
        assertEquals("dtn://testing#123", id.toString());
    }

    public void testEqualsObject() {
        StreamId id1 = new StreamId();
        id1.correlator = 123;
        id1.source = new SingletonEndpoint("dtn://testing");
        
        StreamId id2 = new StreamId();
        id2.correlator = 123;
        id2.source = new SingletonEndpoint("dtn://testing");
        
        assertEquals(id1, id2);
    }

}
