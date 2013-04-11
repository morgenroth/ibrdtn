package ibrdtn.example.api;

import ibrdtn.api.APIException;
import ibrdtn.api.EventClient;
import ibrdtn.api.ExtendedClient;
import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.PayloadBlock;
import ibrdtn.example.MessageData;
import ibrdtn.example.callback.CallbackHandler;
import ibrdtn.example.callback.ICallback;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * A thin wrapper around the IBR-DTN Java Library that takes care of connecting to the daemon and provides some
 * convenience functions.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class DTNClient {

    private static final Logger logger = Logger.getLogger(DTNClient.class.getName());
    private ExecutorService executor;
    private ExtendedClient exClient = null;
    private EventClient eventClient = null;
    private ibrdtn.api.sab.CallbackHandler sabHandler = null;
    private String endpoint = null;

    /**
     *
     * @param endpoint the application's primary EID
     */
    public DTNClient(String endpoint) {

        executor = Executors.newCachedThreadPool();

        this.endpoint = endpoint;

        exClient = new ExtendedClient();

        sabHandler = new SelectiveHandler(exClient, executor);

        exClient.setHandler(sabHandler);
        exClient.setHost(Constants.HOST);
        exClient.setPort(Constants.PORT);

        connect();
    }

    private void connect() {
        try {
            exClient.open();
            logger.log(Level.FINE, "Successfully connected to DTN daemon");

            exClient.setEndpoint(endpoint);
            logger.log(Level.INFO, "Endpoint ''{0}'' registered.", endpoint);

        } catch (APIException e) {
            logger.log(Level.WARNING, "API error: {0}", e.getMessage());
        } catch (IOException e) {
            logger.log(Level.WARNING, "Could not connect to DTN daemon: {0}", e.getMessage());
        }
    }

    public void send(Bundle bundle) {

        logger.log(Level.INFO, "Sending {0}", bundle);

        final Bundle finalBundle = bundle;
        final ExtendedClient finalClient = this.exClient;

        executor.execute(new Runnable() {
            @Override
            public void run() {

                try {
                    finalClient.send(finalBundle);
                } catch (Exception e) {
                    logger.log(Level.SEVERE, "Unable to send bundle", e);
                }
            }
        });
    }

    public void send(Bundle bundle, ICallback callback) {
        logger.log(Level.INFO, "Sending {0}", bundle);

        if (callback != null) {
            final PayloadBlock payload = bundle.getPayloadBlock();
            if (payload != null) {
                try {
                    PipedInputStream in = new PipedInputStream();
                    final PipedOutputStream out = new PipedOutputStream(in);
                    executor.execute(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                payload.getData().writeTo(out);
                            } catch (IOException ex) {
                                logger.log(Level.SEVERE, "Failed writing payload to stream", ex);
                            }
                        }
                    });
                    ObjectInputStream ois = new ObjectInputStream(in);
                    MessageData data = (MessageData) ois.readObject();
                    CallbackHandler.getInstance().add(data.getId(), callback);
                } catch (IOException | ClassNotFoundException ex) {
                    logger.log(Level.SEVERE, "Failed reading payload", ex);
                }
            }
        }

        send(bundle);
    }

    public void shutdown() {

        logger.log(Level.INFO, "Shutting down {0}", endpoint);

        executor.shutdown(); // Disable new tasks from being submitted
        try {

            if (!executor.awaitTermination(8, TimeUnit.SECONDS)) {

                logger.log(Level.FINE, "Thread pool did not terminate on shutdown");

                executor.shutdownNow(); // Cancel currently executing tasks
                // Wait a while for tasks to respond to being cancelled
                if (!executor.awaitTermination(5, TimeUnit.SECONDS)) {
                    logger.log(Level.SEVERE, "Thread pool did not terminate");
                }
            }
        } catch (InterruptedException ie) {

            logger.log(Level.SEVERE, "Shutdown interrupted: {0}", ie.getMessage());
            // (Re-)Cancel if current thread also interrupted
            executor.shutdownNow();
            // Preserve interrupt status
            Thread.currentThread().interrupt();
        }

        try {
            exClient.close();
            if (eventClient != null) {
                eventClient.close();
            }
        } catch (IOException e) {
            logger.log(Level.WARNING, "Could not close connection to the DTN daemon: {0}", e.getMessage());
        }

        logger.log(Level.INFO, "DTN connection closed");
    }

    public void setEvents(boolean eventNotifications) {
        if (eventNotifications) {
            EventNotifier notifier = new EventNotifier();
            eventClient = new EventClient(notifier);
            try {
                eventClient.open();
            } catch (IOException e) {
                logger.log(Level.WARNING, "Could not connect to DTN daemon: {0}", e.getMessage());
            }
        } else {
            if (eventClient != null) {
                try {
                    eventClient.close();
                } catch (IOException e) {
                    logger.log(Level.WARNING, "Closing EventClient connection failed: {0}", e.getMessage());
                }
            }
        }
    }

    public void addStaticCallback(String msgType, ICallback callback) {
        CallbackHandler.getInstance().add(msgType, callback);
    }

    public CallbackHandler getCallbackHandler() {
        return CallbackHandler.getInstance();
    }

    public String getConfiguration() {
        return "DTN Client: " + endpoint + " [" + Constants.HOST + ":" + Constants.PORT + "]";
    }

    public ExtendedClient getEC() {
        return this.exClient;
    }
}