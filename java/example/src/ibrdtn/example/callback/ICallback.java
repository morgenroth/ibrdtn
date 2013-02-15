package ibrdtn.example.callback;

import ibrdtn.example.Envelope;

/**
 * Callback classes need to implement this interface.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public interface ICallback {

    public void messageReceived(Envelope message);
}
