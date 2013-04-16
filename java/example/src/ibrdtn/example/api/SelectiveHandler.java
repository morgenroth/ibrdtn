package ibrdtn.example.api;

import ibrdtn.api.APIException;
import ibrdtn.api.ExtendedClient;
import ibrdtn.api.object.Block;
import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.BundleID;
import ibrdtn.api.object.PayloadBlock;
import ibrdtn.api.sab.Custody;
import ibrdtn.api.sab.StatusReport;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.util.LinkedList;
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

    public SelectiveHandler(ExtendedClient client, ExecutorService executor, PayloadType messageType) {
        this.client = client;
        this.executor = executor;
        this.queue = new LinkedList<>();
        this.messageType = messageType;
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
            try {
                switch (messageType) {
                    case BYTE:
                        readByteArray(payload);
                        break;
                    case OBJECT:
                        readObject(payload);
                        break;
                }
            } catch (Exception ex) {
                logger.log(Level.SEVERE, "Unable to decode payload");
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

    @Override
    protected void loadBundle(BundleID id) {
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
}
