package ibrdtn.api;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public interface APIConnection {
	void open() throws IOException;
	Boolean isConnected();
	Boolean isClosed();
	OutputStream getOutputStream() throws IOException;
	InputStream getInputStream() throws IOException;
	void close() throws IOException;
}
