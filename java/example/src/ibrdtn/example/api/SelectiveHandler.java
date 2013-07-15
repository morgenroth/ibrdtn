package ibrdtn.example.api;

import ibrdtn.api.APIException;
import ibrdtn.api.ExtendedClient;
import ibrdtn.api.object.Block;
import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.BundleID;
import ibrdtn.api.sab.Custody;
import ibrdtn.api.sab.StatusReport;
import ibrdtn.example.MessageData;
import static ibrdtn.example.api.PayloadType.BYTE;
import static ibrdtn.example.api.PayloadType.OBJECT;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.OutputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.util.concurrent.ExecutorService;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * The application's custom handler for Simple API for Bundle Protocol (SAB) events, such as reception of a bundle. The
 * SelectiveHandler only loads bundle meta data at first, thus allowing for selective bundle loading.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class SelectiveHandler extends AbstractAPIHandler {

    private static final Logger logger = Logger.getLogger(SelectiveHandler.class.getName());
    protected Thread t;
    protected MessageData messageData;

    public SelectiveHandler(ExtendedClient client, ExecutorService executor, PayloadType messageType) {
        this.client = client;
        this.executor = executor;
        this.messageType = messageType;
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

    @Override
    public void notify(StatusReport r) {
        logger.log(Level.INFO, r.toString());
    }

    @Override
    public void notify(Custody c) {
        logger.log(Level.INFO, c.toString());
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
         * If payload is not to be loaded, markDelivered();
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
            markDelivered();
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

            // echter Thread: daten rauslesen
            t = new Thread(
                    new Runnable() {
                @Override
                public void run() {

                    switch (messageType) {

                        case BYTE:
                            byte[] bytes = null;
                            ByteArrayOutputStream buffer = null;

                            try {

                                buffer = new ByteArrayOutputStream();

                                int nRead;
                                byte[] data = new byte[16384];

                                while ((nRead = is.read(data, 0, data.length)) != -1) {
                                    buffer.write(data, 0, nRead);
                                }

                                buffer.flush();

                                bytes = buffer.toByteArray();

                                StringBuilder sb = new StringBuilder();

                                for (byte b : bytes) {
                                    sb.append(String.format("%02X ", b));
                                }

                                logger.log(Level.INFO, "Payload received: \n\t{0} [{1}]",
                                        new Object[]{sb.toString(), new String(bytes)});
                            } catch (IOException ex) {
                                logger.log(Level.SEVERE, "Unable to decode payload");
                            }
                            break;

                        case OBJECT:
                            ObjectInputStream ois = null;
                            try {
                                ois = new ObjectInputStream(is);

                                Object object = ois.readObject();

                                // Prüfen ob die Nachricht vom Typ XMLData ist
                                String messageType = object.getClass().getCanonicalName();
                                if (messageType.equals(MessageData.class.getCanonicalName())) {

                                    messageData = (MessageData) object;

                                } else {
                                    logger.log(Level.WARNING, "Unknown Message Type ''{0}''", messageType);
                                }

                            } catch (IOException | ClassNotFoundException ex) {
                                logger.log(Level.SEVERE, "Unable to decode payload");
                            } finally {
                                try {
                                    if (ois != null) {
                                        ois.close();
                                    }
                                } catch (IOException ex) {
                                    logger.log(Level.SEVERE, "Failed to close streams", ex);
                                }
                            }
                            break;
                    }
                }
            });
            t.start();

        } catch (IOException ex) {
            logger.log(Level.SEVERE, "Opening pipes failed", ex);
        }
        return os;
    }

    @Override
    public void endPayload() {
        if (os != null) {
            try {
                // join auf oben gestarteten Thread
                t.join();
            } catch (InterruptedException ex) {
                logger.log(Level.SEVERE, null, ex);
            } finally {
                try {
                    if (os != null) {
                        os.close();
                    }
                } catch (IOException ex) {
                    logger.log(Level.SEVERE, "Failed to close streams", ex);
                }
            }
        }
        markDelivered();
    }

    @Override
    public void progress(long pos, long total) {
        logger.log(Level.INFO, "Payload: {0} of {1} bytes", new Object[]{pos, total});
    }
}
