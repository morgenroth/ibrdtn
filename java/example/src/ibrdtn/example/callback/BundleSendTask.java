package ibrdtn.example.callback;

import ibrdtn.api.object.Bundle;

/**
 * Wraps callback responses for asynchronous sending.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class BundleSendTask {

    private Bundle bundle;
    private ICallback callback;

    public BundleSendTask(Bundle bundle, ICallback callback) {
        this.bundle = bundle;
        this.callback = callback;
    }

    public Bundle getBundle() {
        return bundle;
    }

    public void setBundle(Bundle bundle) {
        this.bundle = bundle;
    }

    public ICallback getCallback() {
        return callback;
    }

    public void setCallback(ICallback callback) {
        this.callback = callback;
    }
}
