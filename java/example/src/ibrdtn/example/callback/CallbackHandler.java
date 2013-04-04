package ibrdtn.example.callback;

import ibrdtn.example.Envelope;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class CallbackHandler {

    private static final Logger logger = Logger.getLogger(CallbackHandler.class.getName());
    private static CallbackHandler instance;
    private static Map<String, ICallback> callbacks;
    private BlockingQueue<Envelope> incomingMessages = new LinkedBlockingQueue<>();

    public static CallbackHandler getInstance() {
        if (instance == null) {
            instance = new CallbackHandler();
            callbacks = new HashMap<>();
        }
        return instance;
    }

    private CallbackHandler() {
    }

    /**
     * Forwards a given message to registered callback components, if any.
     *
     * @param message
     */
    public void forwardMessage(Envelope message) {
        try {
            incomingMessages.put(message);
        } catch (InterruptedException ex) {
            logger.log(Level.SEVERE, "[Error] Saving incoming message failed", ex);
        }


        String inResponseTo = message.getData().getCorrelationId();
        boolean processed = false;

        ICallback callback = get(inResponseTo);

        /*
         * Process message-specific rules.
         */
        if (callback != null) {
            logger.log(Level.FINE, "Dynamic callback. Message received in response to [{0}]", inResponseTo);
            callback.messageReceived(message);
            processed = true;
        }

        /*
         * Process static, i.e., type-specific, rules.
         */
        callback = get(message.getClass().getCanonicalName());
        if (callback != null) {
            logger.log(Level.FINE, "Static callback. Message type ''{0}''", message.getClass().getName());
            callback.messageReceived(message);
            processed = true;
        }

        if (!processed) {
            logger.log(Level.WARNING, "Bundle ({0}) discarded---no according CallbackHandler found.",
                    message.getBundleID());
        }
    }

    public void add(String id, ICallback handler) {
        callbacks.put(id, handler);
    }

    public ICallback get(String id) {
        return callbacks.get(id);
    }

    @Deprecated
    public Envelope takeIncomingMessage() throws InterruptedException {
        Envelope env = incomingMessages.take();

        return env;
    }
}
