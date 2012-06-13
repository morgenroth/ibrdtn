/*
 * UnixAPIConnection.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
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
