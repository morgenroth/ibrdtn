package de.tubs.ibr.dtn.service;

import java.io.IOException;
import java.io.OutputStream;

public class CountingOutputStream extends OutputStream {

    private final OutputStream proxy;
    private long count = 0;

    public CountingOutputStream(final OutputStream out) {
        assert (out != null);
        this.proxy = out;
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
        proxy.write(b);
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException {
        count += len;
        proxy.write(b, off, len);
    }

    @Override
    public void write(byte[] b) throws IOException {
        count += b.length;
        proxy.write(b);
    }

    @Override
    public void close() throws IOException {
        proxy.close();
    }

    @Override
    public void flush() throws IOException {
        proxy.flush();
    }
}