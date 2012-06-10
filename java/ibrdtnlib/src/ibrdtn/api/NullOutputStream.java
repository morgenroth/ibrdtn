package ibrdtn.api;

import java.io.IOException;
import java.io.OutputStream;

public class NullOutputStream extends OutputStream {

    private long count = 0;

    public NullOutputStream() {
    }

    public void reset() {
        this.count = 0;
    }

    public long getCount() {
        if(count >= 0 && count <= Long.MAX_VALUE) {
            return count;
        }
        throw new IllegalStateException("out bytes exceeds Long.MAX_VALUE: " + count);
    }

    @Override
    public void write(int b) throws IOException {
        ++count; // just 1 byte is wrote
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException {
        count += len;
    }

    @Override
    public void write(byte[] b) throws IOException {
        count += b.length;
    }

    @Override
    public void close() throws IOException {
    }

    @Override
    public void flush() throws IOException {
    }
}