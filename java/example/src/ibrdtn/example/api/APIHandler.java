package ibrdtn.example.api;

import ibrdtn.api.ExtendedClient;
import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.BundleID;
import ibrdtn.api.object.PayloadBlock;
import ibrdtn.example.Envelope;
import ibrdtn.example.MessageData;
import ibrdtn.example.PayloadType;
import ibrdtn.example.callback.CallbackHandler;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.util.Queue;
import java.util.concurrent.ExecutorService;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public abstract class APIHandler implements ibrdtn.api.sab.CallbackHandler {

    private static final Logger logger = Logger.getLogger(APIHandler.class.getName());
    protected PipedInputStream is;
    protected PipedOutputStream os;
    protected ExtendedClient client;
    protected ExecutorService executor;
    protected BundleID bundleID = new BundleID();
    protected Bundle bundle = null;
    protected Queue<BundleID> queue;
    protected boolean processing;
    protected PayloadType messageType;

    protected void readByteArray(PayloadBlock payload) throws IOException {

        byte[] bytes = new byte[16];
        int readBytes = is.read(bytes);

        StringBuilder sb = new StringBuilder();

        for (byte b : bytes) {
            sb.append(String.format("%02X ", b));
        }

        logger.log(Level.INFO, "Payload received: \n\t{0} ({1} bytes) [{2}]",
                new Object[]{sb.toString(), readBytes, new String(bytes)});

        if (payload != null) {
            payload.setData(bytes);
        }
    }

    protected void readObject(PayloadBlock payload) throws IOException, ClassNotFoundException {

        ObjectInputStream ois = new ObjectInputStream(is);
        MessageData data = (MessageData) ois.readObject();

        //logger.log(Level.INFO, "Payload received: \n{0}", data);

        if (payload != null) {
            payload.setData(data);
        }

        forwardMessage(data);
    }

    protected void forwardMessage(MessageData data) {
        Envelope envelope = new Envelope();
        envelope.setBundleID(bundleID);
        envelope.setData(data);

        logger.log(Level.INFO, "Data received: {0}", envelope);

        CallbackHandler.getInstance().forwardMessage(envelope);
    }

    protected void endProcessingOfCurrentBundle() {
        markDelivered();
        processing = false;
        if (!queue.isEmpty()) {
            loadBundle(queue.poll());
        }
    }

    protected void markDelivered() {
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

    protected abstract void loadBundle(BundleID poll);
}
