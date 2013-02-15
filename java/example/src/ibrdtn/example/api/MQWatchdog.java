package ibrdtn.example.api;

import ibrdtn.example.callback.BundleSendTask;
import ibrdtn.example.callback.CallbackHandler;
import ibrdtn.api.object.Bundle;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Checks the outgoing message queue and asynchronously sends queued messages.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class MQWatchdog implements Runnable {

    private static final Logger logger = Logger.getLogger(MQWatchdog.class.getName());
    private DTNClient client;

    public MQWatchdog(DTNClient client) {
        this.client = client;
    }

    @Override
    public void run() {

        try {
            while (true) {
                BundleSendTask task = CallbackHandler.getInstance().getQueuedSendTask();
                Bundle bundle = task.getBundle();
                logger.log(Level.FINE, "Watchdog: Sending message: ''{0}''", bundle);
                client.send(bundle);
            }
        } catch (InterruptedException e) {
            logger.log(Level.WARNING, "MessageWatchdog has been interrupted");
        }
    }
}
