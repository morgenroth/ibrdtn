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
 * PassthroughHandler directly loads a new bundle after being notified of it.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class PassthroughHandler extends APIHandler {

    private static final Logger logger = Logger.getLogger(PassthroughHandler.class.getName());

    public PassthroughHandler(ExtendedClient client, ExecutorService executor, PayloadType messageType) {
        this.client = client;
        this.executor = executor;
        this.queue = new LinkedList<>();
        this.messageType = messageType;
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
        logger.log(Level.FINE, "Receiving: {0}", bundle);
        this.bundle = bundle;
    }

    @Override
    public void endBundle() {
        logger.log(Level.FINE, "Bundle received");

        endProcessingOfCurrentBundle();
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
    protected void loadBundle(final BundleID id) {
        processing = true;

        this.bundleID = id;

        final ExtendedClient exClient = this.client;
        executor.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    // Load the next bundle
                    exClient.loadAndGetBundle();
                    logger.log(Level.INFO, "New bundle loaded");
                } catch (APIException e) {
                    logger.log(Level.WARNING, "Failed to load next bundle");
                }
            }
        });
    }
}
