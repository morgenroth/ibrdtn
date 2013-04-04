package ibr.timpner.dtn;

import ibrdtn.example.MessageData;
import ibrdtn.example.api.DTNClient;
import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.Bundle.Priority;
import ibrdtn.api.object.EID;
import ibrdtn.api.object.SingletonEndpoint;
import ibrdtn.example.callback.ICallback;
import ibrdtn.example.callback.SimpleCallback;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

/**
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class SimpleTest {

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

    public SimpleTest() {
    }

    @BeforeClass
    public static void setUpClass() {
        dtnClient1 = new DTNClient(EID_1);
//        dtnClient2 = new DTNClient(EID_2, false);
//        dtnClient2.addStaticCallback(Envelope.class.getCanonicalName(), new SimpleCallback());

//        source = new SingletonEndpoint(NODE + EID_1);
        data = new MessageData();
        data.setId("42");
        data.setCorrelationId("41");
    }

    @AfterClass
    public static void tearDownClass() {
        dtnClient1.shutdown();
//        dtnClient2.shutdown();
    }

    @Before
    public void setUp() {
        destination = new SingletonEndpoint(NODE + EID_2);
    }

    @After
    public void tearDown() {
    }

    /**
     * Test of send method, of class DTNClient.
     */
    @Test
    public void testSend() throws Exception {
        Bundle bundle = new Bundle(destination, 3600);
//        bundle.appendBlock(new PayloadBlock(data));

        /*
         * Custom callback definitions.
         */
        ICallback callback = new SimpleCallback();

        dtnClient1.send(bundle, callback);
    }
}
