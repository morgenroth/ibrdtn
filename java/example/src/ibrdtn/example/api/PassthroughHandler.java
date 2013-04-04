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
 * PassthroughHandler directly loads a new bundle after being notified of it.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class PassthroughHandler implements ibrdtn.api.sab.CallbackHandler {

    private static final Logger logger = Logger.getLogger(PassthroughHandler.class.getName());
    private BundleID bundleID = new BundleID();
    private Bundle bundle = null;
    private ExtendedClient client;
    private ExecutorService executor;
    private PipedInputStream is;
    private PipedOutputStream os;
    private Queue<BundleID> queue;
    private boolean processing = false;

    public PassthroughHandler(ExtendedClient client, ExecutorService executor) {
        this.client = client;
        this.executor = executor;
        this.queue = new LinkedList<>();
    }

    @Override
    public void notify(final BundleID id) {
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
        markDelivered();

        processing = false;
        if (!queue.isEmpty()) {
            loadBundle(queue.poll());
        }
    }

    @Override
    public void startBlock(Block block) {
        logger.log(Level.SEVERE, "Receiving new block: {0}", block.toString());
        bundle.appendBlock(block);
    }

    @Override
    public void endBlock() {
        logger.log(Level.SEVERE, "Block received");
    }

    @Override
    public OutputStream startPayload() {
        logger.log(Level.SEVERE, "Receiving payload");
        /*
         * For a detailed description of how different streams affect efficiency, consult:
         * 
         * code.google.com/p/io-tools
         */
        is = new PipedInputStream();
        try {
            os = new PipedOutputStream(is);//new ByteArrayOutputStream();
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

                logger.log(Level.SEVERE, "Payload received: \n{0}", data);

                if (payload != null) {
                    payload.setData(data);
                }
                
                forwardMessage(data);

            } catch (IOException | ClassNotFoundException e) {

                try { // Read byte array from bundle
                    byte[] bytes = new byte[256];
                    is.read(bytes);

                    logger.log(Level.SEVERE, "Payload received: \n\t{0} ({1} bytes)",
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
                } catch (IOException ex) {
                    logger.log(Level.SEVERE, "Failed to close streams", ex);
                }
            }
//                ByteArrayOutputStream baos = (ByteArrayOutputStream) stream;
//                PayloadBlock payload = null;
//                for (Block block : bundle.getBlocks()) {
//                    if (block.getType() == PayloadBlock.type) {
//                        payload = (PayloadBlock) block;
//                    }
//                }
//                payload.setData(baos.toByteArray());
//
//                try { // Try reading a MessageData object from the stream
//                    ByteArrayInputStream in = new ByteArrayInputStream(baos.toByteArray());
//                    ObjectInputStream is = new ObjectInputStream(in);
//                    MessageData messageData = (MessageData) is.readObject();
//                    logger.log(Level.SEVERE, "Payload ''MessageData'' received: {0}", messageData);
//                } catch (IOException | ClassNotFoundException e) {
//                    logger.log(Level.SEVERE, "Payload bytes received: {0}\n{1}",
//                            new Object[]{payload.getData().size() + " bytes", baos.toString()});
//                }

//            } catch (Exception e) {
//                logger.log(Level.WARNING, "Unable to decode payload");
//            }
        }
    }

    @Override
    public void progress(long pos, long total) {
        logger.log(Level.SEVERE, "Payload: {0} of {1} bytes.", new Object[]{pos, total});
    }

    private void loadBundle(final BundleID id) {
        processing = true;

        this.bundleID = id;

        final ExtendedClient exClient = this.client;
        executor.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    /*
                     * Load the next bundle
                     */
                    exClient.loadAndGetBundle();
                    logger.log(Level.SEVERE, "New bundle loaded: {0}", id);
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

    private void forwardMessage(MessageData data) {
        Envelope envelope = new Envelope();
        envelope.setBundleID(bundleID);
        envelope.setData(data);
        
        CallbackHandler.getInstance().forwardMessage(envelope);
    }
}
