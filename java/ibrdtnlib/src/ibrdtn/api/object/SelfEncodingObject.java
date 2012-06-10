package ibrdtn.api.object;

import java.io.IOException;
import java.io.OutputStream;

public interface SelfEncodingObject {
	public void encode64(OutputStream output) throws IOException;
	public Long getLength();
}
