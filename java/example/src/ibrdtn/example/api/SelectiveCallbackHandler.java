package ibrdtn.example.api;

import ibrdtn.api.APIException;
import ibrdtn.api.ExtendedClient;
import ibrdtn.api.object.Block;
import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.BundleID;
import ibrdtn.api.object.PayloadBlock;
import ibrdtn.api.sab.Custody;
import ibrdtn.api.sab.StatusReport;
import java.io.ByteArrayOutputStream;
import java.io.OutputStream;
import java.util.concurrent.ExecutorService;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * The application's custom handler for Simple API for Bundle Protocol (SAB) events, such as reception of a bundle.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class SelectiveCallbackHandler implements ibrdtn.api.sab.CallbackHandler {

    private static final Logger logger = Logger.getLogger(SelectiveCallbackHandler.class.getName());
    private BundleID bundleID = new BundleID();
    private Bundle bundle = null;
    private ExtendedClient client;
    private ExecutorService executor;
    private OutputStream stream;

    public SelectiveCallbackHandler(ExtendedClient client, ExecutorService executor) {
        this.client = client;
        this.executor = executor;
    }

    @Override
    public void startBundle(Bundle bundle) {
        logger.log(Level.SEVERE, "Receiving new bundle: {0}", bundle);
        this.bundle = bundle;
    }

    @Override
    public void endBundle() {
        logger.log(Level.FINE, "Bundle received");

        final BundleID finalBundleID = this.bundleID;
        final ExtendedClient finalClient = this.client;

        // run the queue and delivered process asynchronously
        executor.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    /*
                     * Mark bundle as delivered...
                     */
//                    finalClient.markDelivered(finalBundleID);
                    /*
                     * or, get payload.
                     */
                    finalClient.getPayload();

                } catch (Exception e) {
                    logger.log(Level.SEVERE, "Unable to mark bundle as delivered.", e);
                }
            }
        });
    }

    @Override
    public void startBlock(Block block) {
        logger.log(Level.SEVERE, "New block received: {0}", block.toString());
        bundle.appendBlock(block);
    }

    @Override
    public void endBlock() {
        logger.log(Level.SEVERE, "Ending block");
    }

    @Override
    public OutputStream startPayload() {
        logger.log(Level.SEVERE, "Receiving payload");
        /*
         * For example, use a byte array stream to transfer the data. Alternatively, object streams or file streams 
         * can be used, too.
         */
        stream = new ByteArrayOutputStream();
        return stream;
    }

    @Override
    public void endPayload() {
        if (stream != null) {
            try {
                ByteArrayOutputStream baos = (ByteArrayOutputStream) stream;
                PayloadBlock payloadBlock = (PayloadBlock) bundle.getBlocks().getLast();
                payloadBlock.setData(baos.toByteArray());

                // Example: add message to database
                // Message msg = new Message(received.source, received.destination, playfile);
                // msg.setCreated(received.timestamp);
                // msg.setReceived(new Date());
                // _database.put(Folder.INBOX, msg);
                logger.log(Level.SEVERE, "Payload received: {0}", baos.toString());

                final BundleID finalBundleID = this.bundleID;
                final ExtendedClient finalClient = this.client;

                // run the queue and delivered process asynchronously
                executor.execute(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            /*
                             * Mark bundle as delivered...
                             */
                            finalClient.markDelivered(finalBundleID);
                        } catch (Exception e) {
                            logger.log(Level.SEVERE, "Unable to mark bundle as delivered.", e);
                        }
                    }
                });
            } catch (Exception e) {
                logger.log(Level.WARNING, "Unable to decode payload");
            }
        }
    }

    @Override
    public void progress(long pos, long total) {
        logger.log(Level.SEVERE, "Payload: {0} of {1} bytes.", new Object[]{pos, total});
    }

    @Override
    public void notify(BundleID id) {
        this.bundleID = id;

        final ExtendedClient exClient = this.client;
        executor.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    /*
                     * Load the next bundle...
                     */
                    //exClient.loadAndGetBundle();
                    /*
                     * or, get bundle info and manually select/omit payload.
                     */
                    exClient.loadBundle();
                    exClient.getBundleInfo();
                    logger.log(Level.FINE, "New bundle loaded");
                } catch (APIException e) {
                    logger.log(Level.WARNING, "Failed to load next bundle");
                }
            }
        });
    }

    @Override
    public void notify(StatusReport r) {
        logger.log(Level.SEVERE, r.toString());
    }

    @Override
    public void notify(Custody c) {
        throw new UnsupportedOperationException("Not supported yet.");
    }
}
