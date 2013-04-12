package ibrdtn.example.callback;

import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.PayloadBlock;
import ibrdtn.api.object.SingletonEndpoint;
import ibrdtn.example.ui.DTNExampleApp;
import ibrdtn.example.Envelope;
import ibrdtn.example.MessageData;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * A callback that triggers an automatic response to the message source and prints the message in the GUI.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class AutoResponseCallback implements ICallback {

    private static final Logger logger = Logger.getLogger(AutoResponseCallback.class.getName());
    private final DTNExampleApp gui;

    public AutoResponseCallback(DTNExampleApp gui) {
        this.gui = gui;
    }

    @Override
    public void messageReceived(Envelope message) {

        MessageData data = new MessageData();

        logger.log(Level.INFO, "Building auto-response...");

        // Increment message ID for response
        data.setId(String.valueOf(Integer.parseInt(message.getData().getId()) + 1));
        data.setCorrelationId(message.getData().getId());

        // Send response back to source
        SingletonEndpoint destination = new SingletonEndpoint(message.getBundleID().getSource());
        Bundle bundle = new Bundle(destination, 3600);
        bundle.appendBlock(new PayloadBlock(data));

        gui.getDtnClient().send(bundle);
    }
}