package de.tubs.ibr.dtn.service;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;

public class UnixAPIConnection implements ibrdtn.api.APIConnection {

	private LocalSocket _socket = null;
	private Boolean _closed = false;
	private String _path = null;
	
	public UnixAPIConnection(String path)
	{
		this._socket = new LocalSocket();
		this._path = path;
	}

	@Override
	public synchronized Boolean isConnected() {
		if (_closed) return false;
		return _socket.isConnected();
	}

	@Override
	public synchronized Boolean isClosed() {
		if (_closed) return true;
		try {
			return _socket.isClosed();
		} catch (java.lang.UnsupportedOperationException ex) {
			return true;
		}
	}

	@Override
	public OutputStream getOutputStream() throws IOException {
		return _socket.getOutputStream();
	}

	@Override
	public InputStream getInputStream() throws IOException {
		return _socket.getInputStream();
	}

	@Override
	public synchronized void close() throws IOException {
		if (_closed) return;
		_closed = true;
		_socket.close();
	}

	@Override
	public synchronized void open() throws IOException {
		this._socket.connect( new LocalSocketAddress(this._path, LocalSocketAddress.Namespace.FILESYSTEM) );
		this._closed = false;
	}

}
