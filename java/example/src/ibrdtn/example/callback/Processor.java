package ibrdtn.example.callback;

import ibrdtn.api.ExtendedClient;
import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.PayloadBlock;
import ibrdtn.api.object.SingletonEndpoint;
import ibrdtn.example.Envelope;
import ibrdtn.example.MessageData;
import java.util.concurrent.ExecutorService;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class Processor implements Runnable {

    private static final Logger logger = Logger.getLogger(Processor.class.getName());
    private Envelope envelope;
    private final ExtendedClient client;
    private final ExecutorService executor;

    public Processor(Envelope envelope, ExtendedClient client, ExecutorService executor) {
        this.envelope = envelope;
        this.client = client;
        this.executor = executor;
    }

    @Override
    public void run() {
        processMessage();
    }

    private void processMessage() {

        MessageData data = new MessageData();

        logger.log(Level.INFO, "Building auto-response...");

        // Increment message ID for response
        data.setId(String.valueOf(Integer.parseInt(envelope.getData().getId()) + 1));
        data.setCorrelationId(envelope.getData().getId());

        // Send response back to source
        SingletonEndpoint destination = new SingletonEndpoint(envelope.getBundleID().getSource());
        Bundle bundle = new Bundle(destination, 3600);
        bundle.appendBlock(new PayloadBlock(data));

        logger.log(Level.INFO, "Sending {0}", bundle);

        final Bundle finalBundle = bundle;

        executor.execute(new Runnable() {
            @Override
            public void run() {

                try {
                    client.send(finalBundle);
                } catch (Exception e) {
                    logger.log(Level.SEVERE, "Unable to send bundle", e);
                }
            }
        });
    }
}
