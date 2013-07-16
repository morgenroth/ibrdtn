package ibrdtn.example.callback;

import ibrdtn.example.Envelope;

/**
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class Processor implements Runnable {

    private Envelope envelope;

    public Processor(Envelope envelope) {
        this.envelope = envelope;
    }

    @Override
    public void run() {
        throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }
}
