package ibrdtn.example.api;

import ibrdtn.example.MessageData;
import ibrdtn.example.callback.CallbackHandler;
import ibrdtn.example.callback.ICallback;
import ibrdtn.api.EventClient;
import ibrdtn.api.ExtendedClient;
import ibrdtn.api.ExtendedClient.APIException;
import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.EID;
import ibrdtn.api.object.GroupEndpoint;
import ibrdtn.api.object.PayloadBlock;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.List;
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
    private MySABHandler sabHandler = null;
    private String endpoint = null;

    /**
     *
     * @param endpoint the application's primary EID
     */
    public DTNClient(String endpoint) {

        executor = Executors.newCachedThreadPool();

        this.endpoint = endpoint;

        exClient = new ExtendedClient();

        sabHandler = new MySABHandler(exClient, executor);

        exClient.setHandler(sabHandler);
        exClient.setHost(Constants.HOST);
        exClient.setPort(Constants.PORT);

        connect();

        MQWatchdog watchdog = new MQWatchdog(this);
        executor.execute(watchdog);
    }

    public DTNClient(String endpoint, boolean eventNotifications) {
        this(endpoint);

        if (eventNotifications) {
            EventNotifier notifier = new EventNotifier();
            eventClient = new EventClient(notifier);
            try {
                eventClient.open();
            } catch (UnknownHostException e) {
                logger.log(Level.WARNING, "Could not connect to DTN daemon: {0}", e.getMessage());
            } catch (IOException e) {
                logger.log(Level.SEVERE, "Connection to DTN daemon failed!");
            }
        }
    }

    private void connect() {
        try {
            exClient.open();
            logger.log(Level.FINE, "Successfully connected to DTN daemon.");

            exClient.setEndpoint(endpoint);
            logger.log(Level.INFO, "Endpoint ''{0}'' registered.", endpoint);

        } catch (APIException e) {
            logger.log(Level.WARNING, "API error: {0}", e.toString());
        } catch (UnknownHostException e) {
            logger.log(Level.WARNING, "Could not connect to DTN daemon: {0}", e.getMessage());
        } catch (IOException e) {
            logger.log(Level.SEVERE, "Connection to DTN daemon failed!");
        }
    }

    public void send(Bundle bundle) {

        logger.log(Level.INFO, "Send procflags {0}", bundle.getProcflags());

        final Bundle finalBundle = bundle;
        final ExtendedClient finalClient = this.exClient;

        executor.execute(new Runnable() {
            @Override
            public void run() {

                try {
                    finalClient.send(finalBundle);
                } catch (Exception e) {
                    logger.log(Level.SEVERE, "Unable to send bundle.", e);
                }
            }
        });
    }

    public void send(Bundle bundle, MessageData data, ICallback callback) {

        bundle.appendBlock(new PayloadBlock(data));

        if (callback != null) {
            CallbackHandler.getInstance().add(data.getId(), callback);
        }

        send(bundle);
    }

    /**
     * Adds a registration of the form '$node/$id' to the client connection. The registration will not be used as the
     * source when sending bundles,
     *
     * @see setEndpoint for this purpose
     *
     * @param id the application name that will be concatenated to the daemons EID
     * @throws APIException If the request fails.
     */
    public void addEndpoint(String id) throws APIException {
        exClient.addEndpoint(id);
        logger.log(Level.INFO, "Endpoint ''{0}'' registered.", id);
    }

    /**
     * Gets the primary endpoint for this client connection in the form 'dtn://$node/$id'.
     *
     * @return the primary EID (dtn://$node/$id)
     * @throws ibrdtn.api.ExtendedClient.APIException
     */
    public EID getEndpoint() throws APIException {
        EID eid = exClient.getEndpoint();
        logger.log(Level.INFO, "Endpoint ''{0}''", eid);
        return eid;
    }

    /**
     * Removes a registration of the form '$node/$id' from the client connection.
     *
     * @param id the application name
     * @throws APIException If the request fails.
     */
    public void removeEndpoint(String id) throws APIException {
        exClient.removeEndpoint(id);
        logger.log(Level.INFO, "Endpoint ''{0}'' removed.", id);
    }

    /**
     * Registers a group endpoint in the form gid = 'dtn://$name[/$id]'. This registration therefore is independent from
     * the node name and can be used for group communications.
     *
     * @param gid The GID to register ('dtn://$name[/$id]').
     * @throws APIException If the request fails.
     */
    public void addGID(String gid) throws APIException {
        exClient.addRegistration(new GroupEndpoint(gid));
        logger.log(Level.INFO, "GroupEndpoint ''{0}'' registered.", gid);
    }

    /**
     * Removes a group endpoint registration in the form gid = 'dtn://$name[/$id]' of this client connection.
     *
     * @param gid The GID to remove ('dtn://$name[/$id]').
     * @throws APIException If the request fails.
     */
    public void removeGID(String gid) throws APIException {
        exClient.removeRegistration(new GroupEndpoint(gid));
        logger.log(Level.INFO, "GroupEndpoint ''{0}'' removed.", gid);
    }

    /**
     * Gets all neighbors available to the daemon.
     *
     * @return A list of EIDs. Each of them is an available neighbor.
     * @throws APIException If the request fails.
     */
    public List<String> getNeighbors() throws APIException {
        return exClient.getNeighbors();
    }

    /**
     * Gets the EID of the dtn daemon in the form 'dtn://$name'.
     *
     * @return The daemons EID
     * @throws APIException If the request fails.
     */
    public EID getNodeName() throws APIException {
        return exClient.getNodeName();
    }

    /**
     * Gets all registrations of this connection, including group endpoints.
     *
     * @return A list with EIDs.
     * @throws APIException If the request fails.
     */
    public List<String> getRegistrations() throws APIException {
        return exClient.getRegistration();
    }

    public void shutdown() {

        logger.log(Level.SEVERE, "Shutting down {0}", endpoint);

        try {
            exClient.close();
            if (eventClient != null) {
                eventClient.close();
            }
        } catch (IOException e) {
            logger.log(Level.WARNING, "Could not close connection to the DTN daemon: {0}", e.getMessage());
        }

        executor.shutdown(); // Disable new tasks from being submitted
        try {

            if (!executor.awaitTermination(5, TimeUnit.SECONDS)) {

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

        logger.log(Level.INFO, "DTNClient shutdown.");
    }

    public void addStaticCallback(String msgType, ICallback callback) {
        CallbackHandler.getInstance().add(msgType, callback);
    }

    public CallbackHandler getCallbackHandler() {
        return CallbackHandler.getInstance();
    }

    public String getConfiguration() {
        return "DTN Client: " + endpoint + " [" + Constants.HOST + ":" + Constants.PORT + "] \r\n";
    }
}