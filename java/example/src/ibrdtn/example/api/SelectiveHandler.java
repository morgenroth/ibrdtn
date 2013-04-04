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
import java.util.LinkedList;
import java.util.Queue;
import java.util.concurrent.ExecutorService;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * The application's custom handler for Simple API for Bundle Protocol (SAB) events, such as reception of a bundle. The
 * SelectiveHandler only loads bundle meta data at first, thus allowing for selective bundle loading.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class SelectiveHandler implements ibrdtn.api.sab.CallbackHandler {

    private static final Logger logger = Logger.getLogger(SelectiveHandler.class.getName());
    private BundleID bundleID = new BundleID();
    private Bundle bundle = null;
    private ExtendedClient client;
    private ExecutorService executor;
    private OutputStream stream;
    private Queue<BundleID> queue;
    private boolean processing;

    public SelectiveHandler(ExtendedClient client, ExecutorService executor) {
        this.client = client;
        this.executor = executor;
        this.queue = new LinkedList<>();
    }

    @Override
    public void notify(BundleID id) {
        if (!processing) {
            loadBundle(id);
        } else {
            queue.add(id);
        }
    }

    @Override
    public void notify(StatusReport r) {
        logger.log(Level.SEVERE, r.toString());
    }

    @Override
    public void notify(Custody c) {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void startBundle(Bundle bundle) {
        logger.log(Level.SEVERE, "Receiving new bundle: {0}", bundle);
        this.bundle = bundle;
    }

    @Override
    public void endBundle() {
        logger.log(Level.FINE, "Bundle received");

        final ExtendedClient finalClient = this.client;
        executor.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    /*
                     * Decide if payload is interesting, based on Bundle meta data, e.g., payload length of the 
                     * individual block. If applicable, partial payloads can be requested, as well.
                     */
                    logger.log(Level.SEVERE, "Requesting payload");
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
                for (Block block : bundle.getBlocks()) {
                    if (block.getType() == PayloadBlock.type) {
                        ((PayloadBlock) block).setData(baos.toByteArray());
                    }
                }

                logger.log(Level.SEVERE, "Payload received: {0}", baos.toString());

                markDelivered();

                processing = false;
                if (!queue.isEmpty()) {
                    loadBundle(queue.poll());
                }
            } catch (Exception e) {
                logger.log(Level.WARNING, "Unable to decode payload");
            }
        }
    }

    @Override
    public void progress(long pos, long total) {
        logger.log(Level.SEVERE, "Payload: {0} of {1} bytes.", new Object[]{pos, total});
    }

    private void loadBundle(BundleID id) {
        processing = true;

        this.bundleID = id;

        final ExtendedClient exClient = this.client;
        executor.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    /*
                     * Load bundle into queue and manually inspect bundle info to decide whether or not 
                     * to get the payload.
                     */
                    exClient.loadBundle();
                    exClient.getBundleInfo();
                    logger.log(Level.SEVERE, "New bundle loaded, getting meta data");
                } catch (APIException e) {
                    logger.log(Level.WARNING, "Failed to load next bundle");
                }
            }
        });
    }

    private void markDelivered() {
        final BundleID finalBundleID = this.bundleID;
        final ExtendedClient finalClient = this.client;

        // run the queue and delivered process asynchronously
        executor.execute(new Runnable() {
            @Override
            public void run() {
                // Example: add message to database
                // Message msg = new Message(received.source, received.destination, playfile);
                // msg.setCreated(received.timestamp);
                // msg.setReceived(new Date());
                // _database.put(Folder.INBOX, msg);

                try {
                    /*
                     * Mark bundle as delivered...
                     */
                    logger.log(Level.SEVERE, "Delivered: {0}", finalBundleID);
                    finalClient.markDelivered(finalBundleID);
                } catch (Exception e) {
                    logger.log(Level.SEVERE, "Unable to mark bundle as delivered.", e);
                }
            }
        });
    }
}
