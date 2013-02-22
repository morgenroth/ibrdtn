package ibrdtn.example.api;

import ibrdtn.api.APIException;
import ibrdtn.example.Envelope;
import ibrdtn.example.MessageData;
import ibrdtn.example.callback.CallbackHandler;
import ibrdtn.api.ExtendedClient;
import ibrdtn.api.Timestamp;
import ibrdtn.api.object.BundleID;
import ibrdtn.api.sab.SABException;
import ibrdtn.api.sab.SABHandler;
import java.io.IOException;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * The application's custom handler for Simple API for Bundle Protocol (SAB) events, such as reception of a bundle.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class MySABHandler implements SABHandler {

    private static final Logger logger = Logger.getLogger(MySABHandler.class.getName());
    private boolean readingBundle = false;
    private boolean readingBlock = false;
    private String data = null;
    private Timestamp timestamp = null;
    private BundleID bundleID = new BundleID();
    private ExtendedClient client;
    private ExecutorService executor;

    public MySABHandler(ExtendedClient client, ExecutorService executor) {
        this.client = client;
        this.executor = executor;
    }

    @Override
    public void startStream() {
        logger.log(Level.FINE, "Starting stream.");
    }

    @Override
    public void endStream() {
        logger.log(Level.FINE, "Ending stream.");
    }

    @Override
    public void startBundle() {
        logger.log(Level.FINE, "Starting bundle.");
        readingBundle = true;
    }

    @Override
    public void endBundle() {
        logger.log(Level.FINE, "Ending bundle.");

        this.readingBundle = false;

        if ((this.data != null) && (timestamp != null)) {
            Object receivedObject = null;

            try {

                receivedObject = client.decodeObject(data);

            } catch (IOException | ClassNotFoundException e) {
                logger.log(Level.WARNING, "Unable to decode bundle.");
            }

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
                        finalClient.markDelivered(finalBundleID);
                    } catch (Exception e) {
                        logger.log(Level.SEVERE, "Unable to mark bundle as delivered.", e);
                    }
                }
            });

            if (receivedObject != null) {
                if (receivedObject instanceof MessageData) {
                    MessageData messageData = (MessageData) receivedObject;

                    Envelope envelope = new Envelope();
                    envelope.setData(messageData);
                    envelope.setBundleID(bundleID);

                    logger.log(Level.FINE, "New bundle received and successfully converted: {0}", envelope);

                    CallbackHandler.getInstance().forwardMessage(envelope);
                } else {
                    logger.log(Level.WARNING, "Received unknown data type.");
                }
            }
        }
    }

    @Override
    public void startBlock(Integer type) {
        logger.log(Level.FINE, "Starting block.");
        readingBlock = true;
        if (type == 1) {
            data = new String();
        }
    }

    @Override
    public void endBlock() {
        logger.log(Level.FINE, "Ending block.");
        readingBlock = false;
    }

    @Override
    public void attribute(String keyword, String value) {

        logger.log(Level.FINE, "Attribute: {0}={1}", new Object[]{keyword, value});

        if (readingBundle && (!readingBlock)) {
            if (keyword.equalsIgnoreCase("source")) {
                bundleID.setSource(value);
            } else if (keyword.equalsIgnoreCase("destination")) {
                bundleID.setDestination(value);
            } else if (keyword.equalsIgnoreCase("timestamp")) {
                bundleID.setTimestamp(Long.parseLong(value));
                timestamp = new Timestamp(Long.parseLong(value));
            } else if (keyword.equalsIgnoreCase("sequencenumber")) {
                bundleID.setSequenceNumber(Long.parseLong(value));
            } else if (keyword.equalsIgnoreCase("Processing flags")) {
                logger.log(Level.INFO, "Processing flags: {0}", value);
                bundleID.setProcflags(Long.parseLong(value));
            }
        }
    }

    @Override
    public void characters(String data) throws SABException {
        if (this.data != null) {
            this.data += data;
        }
    }

    @Override
    public void notify(Integer type, String data) {

        switch (type) {
            case 602:
                logger.log(Level.FINE, "New bundle notification {0}", String.valueOf(type));

                final ExtendedClient exClient = this.client;
                executor.execute(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            // load the next bundle
                            exClient.loadAndGetBundle();
                            logger.log(Level.FINE, "New bundle loaded");
                        } catch (APIException e) {
                            logger.log(Level.WARNING, "Failed to load next bundle");
                        }
                    }
                });
                break;
            case 603:
                logger.log(Level.SEVERE, "New notification {0}", String.valueOf(type));
                break;
        }
    }

    @Override
    public void response(Integer type, String data) {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void response(List<String> data) {
        throw new UnsupportedOperationException("Not supported yet.");
    }
}