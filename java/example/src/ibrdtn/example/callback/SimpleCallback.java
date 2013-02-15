package ibrdtn.example.callback;

import ibrdtn.example.Envelope;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * A simple callback that prints the received message on the standard output.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class SimpleCallback implements ICallback {

    private static final Logger logger = Logger.getLogger(SimpleCallback.class.getName());

    public SimpleCallback() {
    }

    @Override
    public void messageReceived(Envelope message) {
        logger.log(Level.SEVERE, "Callback received for {0}", message);
    }
}
