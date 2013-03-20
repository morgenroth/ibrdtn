package ibrdtn.example.callback;

import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.PayloadBlock;
import ibrdtn.api.object.SingletonEndpoint;
import ibrdtn.example.DTNExampleApp;
import ibrdtn.example.Envelope;
import ibrdtn.example.MessageData;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * A callback that triggers an automatic response to the message source and prints the message in the GUI.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class ResponseCallback implements ICallback {

    private static final Logger logger = Logger.getLogger(ResponseCallback.class.getName());
    private final DTNExampleApp gui;

    public ResponseCallback(DTNExampleApp gui) {
        this.gui = gui;
    }

    @Override
    public void messageReceived(Envelope message) {

        gui.print("Callback received for " + message);
        logger.log(Level.FINE, "Callback received for {0}", message);

        MessageData data = new MessageData();

        // Increment message ID for response
        data.setId(String.valueOf(Integer.parseInt(message.getData().getId()) + 1));
        data.setCorrelationId(message.getData().getId());

        // Send response back to source
        SingletonEndpoint destination = new SingletonEndpoint(message.getBundleID().getSource());
        Bundle bundle = new Bundle(destination, 3600);
        bundle.appendBlock(new PayloadBlock(data));
        bundle.setPriority(message.getBundleID().getPriority());

        // Register print only callback for response
        ICallback callback = new DisplayCallback(gui);

        gui.print("Sending response...\n");
        logger.log(Level.FINE, "Enqueuing response");

        /*
         * Calling send() directly would cause a deadlock, thus the message to be send needs to be enqueued and then
         * send asynchronously.
         */
        try {
            CallbackHandler.getInstance().enqueueSendTask(new BundleSendTask(bundle, callback));
        } catch (InterruptedException e) {
            logger.log(Level.WARNING, "ResponseCallback has been interrupted");
        }
    }
}