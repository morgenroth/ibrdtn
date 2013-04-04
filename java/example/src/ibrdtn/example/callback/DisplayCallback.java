package ibrdtn.example.callback;

import ibrdtn.example.DTNExampleApp;
import ibrdtn.example.Envelope;
import java.util.logging.Logger;

/**
 * A simple callback that prints the received message in the GUI's text area.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class DisplayCallback implements ICallback {

    private static final Logger logger = Logger.getLogger(DisplayCallback.class.getName());
    private final DTNExampleApp gui;

    public DisplayCallback(DTNExampleApp gui) {
        this.gui = gui;
    }

    @Override
    public void messageReceived(Envelope message) {
        gui.print("Bundle received: " + message);
    }
}
