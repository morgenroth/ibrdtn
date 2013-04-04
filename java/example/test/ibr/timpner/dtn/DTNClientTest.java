package ibr.timpner.dtn;

import ibrdtn.example.Envelope;
import ibrdtn.example.MessageData;
import ibrdtn.example.api.DTNClient;
import ibrdtn.example.callback.SimpleCallback;
import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.Bundle.Priority;
import ibrdtn.api.object.EID;
import ibrdtn.api.object.PayloadBlock;
import ibrdtn.api.object.SingletonEndpoint;
import org.junit.After;
import org.junit.AfterClass;
import static org.junit.Assert.*;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

/**
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class DTNClientTest {

    private static final String NODE = "dtn://timpner-lx/";
    private static final String EID_1 = "client-1";
    private static final String EID_2 = "client-2";
    private static DTNClient dtnClient1;
    private static DTNClient dtnClient2;
    private static EID source;
    private static EID destination;
    private static MessageData data;
    private static Priority priority;
    private static long procflags;

    public DTNClientTest() {
    }

    @BeforeClass
    public static void setUpClass() {
        dtnClient1 = new DTNClient(EID_1);
        dtnClient2 = new DTNClient(EID_2);
        dtnClient2.addStaticCallback(Envelope.class.getCanonicalName(), new SimpleCallback());

        source = new SingletonEndpoint(NODE + EID_1);
        data = new MessageData();
        data.setId("42");
        data.setCorrelationId("41");
    }

    @AfterClass
    public static void tearDownClass() {
        dtnClient1.shutdown();
        dtnClient2.shutdown();
    }

    @Before
    public void setUp() {
        destination = new SingletonEndpoint(NODE + EID_2);
        priority = Priority.NORMAL;
        System.err.println("----------------------------------------------------");
    }

    @After
    public void tearDown() {
    }

    /**
     * Test of send method, of class DTNClient.
     */
    @Test
    public void testCompression() throws Exception {
        System.err.println("--- Compression Test ---");
        Bundle bundle = new Bundle(destination, 3600);
        bundle.appendBlock(new PayloadBlock(data));
        bundle.setFlag(Bundle.Flags.COMPRESSION_REQUEST, true);

        procflags = bundle.getProcflags();

        dtnClient1.send(bundle);
        checkReceivedMessage();
    }

    /**
     * Test of send method, of class DTNClient.
     */
    @Test
    public void testPriority_normal() throws Exception {
        System.err.println("--- Priority Test ---");
        System.err.println("Priority: " + priority);

        Bundle bundle = new Bundle(destination, 3600);
        bundle.appendBlock(new PayloadBlock(data));

        procflags = bundle.getProcflags();

        dtnClient1.send(bundle);
        checkReceivedMessage();
    }

    /**
     * Test of send method, of class DTNClient.
     */
    @Test
    public void testPriority_bulk() throws Exception {
        System.err.println("--- Priority Test ---");
        priority = Priority.BULK;
        System.err.println("Priority: " + priority);

        Bundle bundle = new Bundle(destination, 3600);
        bundle.appendBlock(new PayloadBlock(data));
        bundle.setPriority(priority);

        procflags = bundle.getProcflags();

        dtnClient1.send(bundle);
        checkReceivedMessage();
    }

    /**
     * Test of send method, of class DTNClient.
     */
    @Test
    public void testPriority_expedited() throws Exception {
        System.err.println("--- Priority Test ---");
        priority = Priority.EXPEDITED;
        System.err.println("Priority: " + priority);

        Bundle bundle = new Bundle(destination, 3600);
        bundle.appendBlock(new PayloadBlock(data));
        bundle.setPriority(priority);

        procflags = bundle.getProcflags();

        dtnClient1.send(bundle);
        checkReceivedMessage();
    }

    /**
     * Test of send method, of class DTNClient.
     */
    @Test
    public void testSingleton_true() throws Exception {
        System.err.println("--- Singleton Test ---");
        Bundle bundle = new Bundle(destination, 3600);
        bundle.appendBlock(new PayloadBlock(data));

        procflags = bundle.getProcflags();

        dtnClient1.send(bundle);

        Envelope envelope = dtnClient2.getCallbackHandler().takeIncomingMessage();
        assertTrue(envelope.getBundleID().isSingleton());
    }

    /**
     * Test of send method, of class DTNClient.
     */
    @Test
    public void testReportTo() throws Exception {
        System.err.println("--- Report Test ---");
        Bundle bundle = new Bundle(destination, 3600);
        bundle.appendBlock(new PayloadBlock(data));
        bundle.setReportto(source);
        bundle.setFlag(Bundle.Flags.CUSTODY_REQUEST, true);
        bundle.setFlag(Bundle.Flags.CUSTODY_REPORT, true);
        bundle.setFlag(Bundle.Flags.DELETION_REPORT, true);
        bundle.setFlag(Bundle.Flags.RECEPTION_REPORT, true);
        bundle.setFlag(Bundle.Flags.FORWARD_REPORT, true);
        bundle.setFlag(Bundle.Flags.DELIVERY_REPORT, true);

        procflags = bundle.getProcflags();

        dtnClient1.send(bundle);

        Envelope envelope = dtnClient2.getCallbackHandler().takeIncomingMessage();
        assertTrue(envelope.getBundleID().isSingleton());
    }

    /**
     * Test of send method, of class DTNClient.
     */
//    @Test
//    public void testSingleton_false() throws Exception {
//        String group = "dtn://group-2";
//        dtnClient2.addGID(group);
//        destination = new GroupEndpoint(group);
//
//        Bundle bundle = new Bundle(destination, 3600);
//        bundle.appendBlock(new PayloadBlock(data));
//
//        procflags = bundle.getProcflags();
//
//        dtnClient1.send(bundle);
//
//        Envelope envelope = dtnClient2.getCallbackHandler().takeIncomingMessage();
//        assertFalse(envelope.getBundleID().isSingleton());
//    }
    private void checkReceivedMessage() throws InterruptedException {
        Envelope envelope = dtnClient2.getCallbackHandler().takeIncomingMessage();

        assertEquals(priority, envelope.getBundleID().getPriority());
        assertEquals(source.toString(), envelope.getBundleID().getSource().toString());
        assertEquals(data.getId(), envelope.getData().getId());
        assertEquals(envelope.getData().getId(), data.getId());
        assertEquals(procflags, envelope.getBundleID().getProcflags());
    }
}
