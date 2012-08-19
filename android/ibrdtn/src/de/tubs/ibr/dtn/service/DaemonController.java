package de.tubs.ibr.dtn.service;

import ibrdtn.api.APIConnection;
import ibrdtn.api.ManageClient;
import ibrdtn.api.object.SingletonEndpoint;

import java.io.IOException;
import java.net.UnknownHostException;
import java.util.List;

import android.util.Log;

public class DaemonController {
	
	private final static String TAG = "DaemonController";
	private APIConnection _conn = null;
	private ManageClient _client = null;
	
	public DaemonController(APIConnection connection) {
		_conn = connection;
	}
	
	public void initialize() throws UnknownHostException, IOException {
		_client = new ManageClient();
		_client.setConnection( _conn );
		Log.i("DaemonProcess", "try to connect to daemon");
		_client.open();
	}
	
	public void terminate() { 
		
	}
	
	private ManageClient getConnection() throws UnknownHostException, IOException
	{
		if (_client == null) {
			initialize();
		}
		
		// check existing connection first
		if (_client.isClosed())
		{
			_conn.close();
			Log.i(TAG, "connection to daemon closed");
			throw new IOException("connection has been closed");
		}
		
		return _client;
	}
	
	public void setSuspend()
	{
		try {
			ManageClient c = getConnection();
			c.suspend();
		} catch (UnknownHostException e) {
		} catch (IOException e) {
		}
	}
	
	public void setResume()
	{
		try {
			ManageClient c = getConnection();
			c.resume();
		} catch (UnknownHostException e) {
		} catch (IOException e) {
		}
	}
	
	public List<String> getLog()
	{
		 try {
			 ManageClient c = getConnection();
			 return c.getLog();
		} catch (UnknownHostException e1) {
		} catch (IOException e1) {
		}

		return null;
	}
	
	public List<String> getNeighbors()
	{
		 try {
			 ManageClient c = getConnection();
			 return c.getNeighbors();
		} catch (UnknownHostException e1) {
		} catch (IOException e1) {
		}

		return null;
	}
	
	public void addConnection(SingletonEndpoint eid, String protocol, String address, String port) {
		 try {
			 ManageClient c = getConnection();
			 c.addConnection(eid, protocol, address, port);
		} catch (UnknownHostException e1) {
		} catch (IOException e1) {
		}
	}
	
	public void removeConnection(SingletonEndpoint eid, String protocol, String address, String port) {
		 try {
			 ManageClient c = getConnection();
			 c.removeConnection(eid, protocol, address, port);
		} catch (UnknownHostException e1) {
		} catch (IOException e1) {
		}
	}
}
