package ibrdtn.example.api;

import ibrdtn.api.APIException;
import ibrdtn.api.ExtendedClient;
import ibrdtn.api.object.Block;
import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.BundleID;
import ibrdtn.api.object.PayloadBlock;
import ibrdtn.api.sab.Custody;
import ibrdtn.api.sab.StatusReport;
import ibrdtn.example.Envelope;
import ibrdtn.example.MessageData;
import ibrdtn.example.callback.CallbackHandler;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.OutputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
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
    private PipedInputStream is;
    private PipedOutputStream os;
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
        logger.log(Level.FINE, "Receiving: {0}", bundle);
        this.bundle = bundle;
    }

    @Override
    public void endBundle() {
        logger.log(Level.INFO, "Bundle info received");

        /*
         * Decide if payload is interesting, based on bundle meta data, e.g., payload length of the 
         * individual block. If applicable, partial payloads can be requested, as well. 
         * 
         * If payload is not to be loaded, set processing = false and markDelivered();
         */

        boolean isPayloadInteresting = true;

        if (isPayloadInteresting) {
            final ExtendedClient finalClient = this.client;
            executor.execute(new Runnable() {
                @Override
                public void run() {
                    try {
                        logger.log(Level.INFO, "Requesting payload");
                        finalClient.getPayload();
                    } catch (Exception e) {
                        logger.log(Level.SEVERE, "Unable to mark bundle as delivered.", e);
                    }
                }
            });
        } else {
            endProcessingOfCurrentBundle();
        }
    }

    @Override
    public void startBlock(Block block) {
        //logger.log(Level.FINE, "Receiving: {0}", block.toString());
        bundle.appendBlock(block);
    }

    @Override
    public void endBlock() {
        //logger.log(Level.FINE, "Ending block");
    }

    @Override
    public OutputStream startPayload() {
        //logger.log(Level.INFO, "Receiving payload");
        /*
         * For a detailed description of how different streams affect efficiency, consult:
         * 
         * code.google.com/p/io-tools
         */
        is = new PipedInputStream();
        try {
            os = new PipedOutputStream(is);
        } catch (IOException ex) {
            logger.log(Level.SEVERE, "Opening pipes failed", ex);
        }
        return os;
    }

    @Override
    public void endPayload() {
        if (os != null) {
            PayloadBlock payload = bundle.getPayloadBlock();

            try { // Read object from bundle

                ObjectInputStream ois = new ObjectInputStream(is);
                MessageData data = (MessageData) ois.readObject();

                //logger.log(Level.INFO, "Payload received: \n{0}", data);

                if (payload != null) {
                    payload.setData(data);
                }

                forwardMessage(data);

            } catch (IOException | ClassNotFoundException e) {

                try { // Read byte array from bundle
                    byte[] bytes = new byte[256];
                    is.read(bytes);

                    logger.log(Level.INFO, "Payload received: \n\t{0} ({1} bytes)",
                            new Object[]{new String(bytes), bytes.length});

                    if (payload != null) {
                        payload.setData(bytes);
                    }

                } catch (IOException ex) {
                    logger.log(Level.SEVERE, "Unable to decode payload");
                }
            } finally {
                try {
                    is.close();
                    os.close();
                    endProcessingOfCurrentBundle();
                } catch (IOException ex) {
                    logger.log(Level.SEVERE, "Failed to close streams", ex);
                }
            }
        }
    }

    @Override
    public void progress(long pos, long total) {
        logger.log(Level.INFO, "Payload: {0} of {1} bytes", new Object[]{pos, total});
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
                    logger.log(Level.INFO, "New bundle loaded, getting meta data");
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
                    // Mark bundle as delivered...                    
                    logger.log(Level.FINE, "Delivered: {0}", finalBundleID);
                    finalClient.markDelivered(finalBundleID);
                } catch (Exception e) {
                    logger.log(Level.SEVERE, "Unable to mark bundle as delivered.", e);
                }
            }
        });
    }

    private void forwardMessage(MessageData data) {
        Envelope envelope = new Envelope();
        envelope.setBundleID(bundleID);
        envelope.setData(data);

        logger.log(Level.INFO, "Data received: {0}", envelope);

        CallbackHandler.getInstance().forwardMessage(envelope);
    }

    private void endProcessingOfCurrentBundle() {
        processing = false;
        markDelivered();
        if (!queue.isEmpty()) {
            loadBundle(queue.poll());
        }
    }
}
