package ibrdtn.api;

import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.BundleID;
import ibrdtn.api.object.EID;
import ibrdtn.api.object.GroupEndpoint;
import ibrdtn.api.object.InputStreamBlockData;
import ibrdtn.api.object.PayloadBlock;
import ibrdtn.api.object.PlainSerializer;
import ibrdtn.api.object.SelfEncodingObject;
import ibrdtn.api.object.SelfEncodingObjectBlockData;
import ibrdtn.api.object.SingletonEndpoint;
import ibrdtn.api.sab.Response;
import ibrdtn.api.sab.SABException;
import ibrdtn.api.sab.SABHandler;
import ibrdtn.api.sab.SABParser;

import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.UnknownHostException;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.SynchronousQueue;


public class ExtendedClient extends Client {
	
	private Object connection_mutex = new Object();
	private BufferedWriter _writer = null;
	private DataReceiver _receiver = null;
	
	private Boolean _debug = false;
	
	private Object handler_mutex = new Object();
	protected SABHandler handler = null;
	
	private EID remoteEID = null;
	
	private enum State
	{
		UNINITIALIZED,
		CONNECTING,
		CONNECTED,
		CLOSED,
		CLOSING,
		FAILED
	}
	
	private Object state_mutex = new Object();
	private State state = State.UNINITIALIZED;
	
	private void setState(State s) {
		synchronized(state_mutex) {
			state = s;
			state_mutex.notifyAll();
		}
	}
	
	@Override
	public synchronized Boolean isConnected() {
		synchronized(state_mutex) {
    		try {
	    		while (state == State.CONNECTING)
	    		{
	    			state_mutex.wait();
	    		}
    		} catch (InterruptedException e) {
    			return false;
    		}
    		
    		return (state == State.CONNECTED);
		}
	}

	@Override
	public synchronized Boolean isClosed() {
		synchronized(state_mutex) {
    		try {
	    		while (state == State.CONNECTING)
	    		{
	    			state_mutex.wait();
	    		}
    		} catch (InterruptedException e) {
    			return false;
    		}
    		
    		return (state != State.CONNECTED);
		}
	}

	/**
	 * This exception is thrown if something went wrong during an API call.
	 */
	public class APIException extends Exception
	{
		private static final long serialVersionUID = 7146079367557056047L;
		
		public APIException(String what)
		{
			super(what);
		}
	}

	/**
	 * Create an instance of the extended client interface.
	 * By default this is not connected to a daemon.
	 */
	public ExtendedClient()
	{
		super();
	}
	
	/**
	 * Enable / disable the debug output.
	 * @param val
	 */
	public void setDebug(Boolean val)
	{
		this._debug = val;
	}
	
	private void debug(String msg)
	{
		if (!this._debug) return;
		System.out.println("Debug: " + msg);
	}
	
//	/**
//	 * Get the assigned handler for incoming API events.
//	 * @return The assigned handler of this instance. Null, if no handler is set.
//	 */
//	public SABHandler getHandler() {
//		return handler;
//	}

	/**
	 * Set the handler for incoming API events.
	 * @param handler A handler object derived from SABHandler.
	 */
	public void setHandler(SABHandler handler) {
		synchronized(handler_mutex) {
			this.handler = handler;
		}
	}
	
	/*
	 * (non-Javadoc)
	 * @see ibrdtn.api.Client#open()
	 */
	@Override
	public void open() throws UnknownHostException, IOException
	{
		synchronized(state_mutex) {
			// just return if the connection is already open
			if (state != State.UNINITIALIZED)
				throw new IOException("client in invalid state for open()");
			setState(State.CONNECTING);
		}
		
		try {
			super.open();
			
			synchronized(connection_mutex) {
				this._writer = new BufferedWriter( new OutputStreamWriter(this.ostream) );
			}
		
			// run the notify receiver
			_receiver = new DataReceiver(this);
			_receiver.start();
		
			// switch to event protocol
			if (query("protocol extended") != 200)
			{
				// error
				throw new APIException("protocol switch failed");
			}
		} catch (APIException ex) {
			_writer.close();
			_writer = null;
			_receiver.abort();
			_receiver = null;
			setState(State.UNINITIALIZED);
			throw new IOException(ex.getMessage());
		} catch (UnknownHostException e) {
			setState(State.FAILED);
			throw e;
		} catch (IOException e) {
			setState(State.UNINITIALIZED);
			throw e;
		}
		
		// set state to connected
		setState(State.CONNECTED);
	}
	
	/*
	 * (non-Javadoc)
	 * @see ibrdtn.api.Client#close()
	 */
	@Override
	public void close() throws IOException {
		synchronized(state_mutex) {
    		try {
	    		while (state == State.CONNECTING)
	    		{
	    			state_mutex.wait();
	    		}
    		} catch (InterruptedException e) {
    			setState(State.FAILED);
    			return;
    		}
    		
			// just return if the connection is not open
			if (state != State.CONNECTED) return;
			setState(State.CLOSING);
		}
		
		try {
			synchronized(connection_mutex) {
				if (_writer != null) _writer.close();
				if (_receiver != null) { _receiver.abort(); _receiver = null; }
				super.close();
			}
		} catch (IOException e) {
			setState(State.FAILED);
			throw e;
		}
		
		// set state to connected
		setState(State.CLOSED);
	}
	
	private void mark_error()
	{
		setState(State.FAILED);
		
		try {
			synchronized(connection_mutex) {
				if (_writer != null) _writer.close();
				if (_receiver != null) { _receiver.abort(); _receiver = null; }
				super.close();
			}
		} catch (IOException e) { }
	}
	
	/**
	 * Execute a NOOP command to the API. Just checking that this connection is active.
	 * @throws APIException If the connection has been terminated or the received response is wrong. 
	 */
	public synchronized void noop() throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// send a message with no effect
		send("noop");
		
		// read answer
		if (_receiver.getResponse().getCode() != 400)
		{
			// error
			throw new APIException("noop failed");
		}
	}
	
	/**
	 * Send a bundle directly to the daemon.
	 * @param bundle The bundle to send.
	 * @throws APIException If the transmission fails.
	 */
	public synchronized void send(Bundle bundle) throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// clear the previous bundle first
		if (query("bundle clear") != 200) throw new APIException("bundle clear failed");
		
		// announce a proceeding plain bundle
		if (query("bundle put plain") != 100) throw new APIException("bundle put failed");
		
		PlainSerializer serializer = new PlainSerializer(ostream);

		try {
			serializer.serialize(bundle);
		} catch (IOException e) {
			throw new APIException("serialization of bundle failed.");
		}

		if (_receiver.getResponse().getCode() != 200)
		{
			throw new APIException("bundle rejected or put failed");
		}
		
		// send the bundle away
		if (query("bundle send") != 200) throw new APIException("bundle send failed");
	}
	
	/**
	 * Send a bundle directly to the daemon. The given payload has to be encoded as base64 first.
	 * @param destination The destination of the bundle.
	 * @param lifetime The lifetime of the bundle.
	 * @param base64 The base64 encoded payload.
	 * @param length The length of the un-encoded payload.
	 * @throws APIException If the transmission fails.
	 */
	@Deprecated
	public synchronized void send(EID destination, Integer lifetime, String base64, Long length) throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// clear the previous bundle first
		if (query("bundle clear") != 200) throw new APIException("bundle clear failed");
		
		// announce a proceeding plain bundle
		if (query("bundle put plain") != 100) throw new APIException("bundle put failed");
		
		// upload bundle to the daemon
		if (destination instanceof GroupEndpoint)
		{
			startBundle(0, destination, lifetime, 1);			
		}
		else
		{
			startBundle(16, destination, lifetime, 1);
		}
		
		startBlock(1, length, true);
		
		send(base64);
		send("");
		
		if (_receiver.getResponse().getCode() != 200)
		{
			// error
			throw new APIException("bundle rejected or put failed");
		}
		
		// send the bundle away
		if (query("bundle send") != 200) throw new APIException("bundle send failed");
	}
	
	/**
	 * Send a bundle directly to the daemon. The given payload has to be encoded as base64 first.
	 * @param destination The destination of the bundle.
	 * @param lifetime The lifetime of the bundle.
	 * @param base64 The base64 encoded payload.
	 * @param length The length of the un-encoded payload.
	 * @throws APIException If the transmission fails.
	 */
	@Deprecated
	public void send(EID destination, Integer lifetime, String base64, Integer length) throws APIException
	{
		send(destination, lifetime, base64, new Long(length));
	}
	
	/**
	 * Send a bundle directly to the daemon.
	 * @param destination The destination of the bundle.
	 * @param lifetime The lifetime of the bundle.
	 * @param data The payload as byte-array.
	 * @throws APIException If the transmission fails.
	 */
	public void send(EID destination, Integer lifetime, byte[] data) throws APIException
	{
		send(destination, lifetime, data, Bundle.Priority.NORMAL);
	}
	
	/**
	 * Send a bundle directly to the daemon.
	 * @param destination The destination of the bundle.
	 * @param lifetime The lifetime of the bundle.
	 * @param data The payload as byte-array.
	 * @param priority The priority of the bundle
	 * @throws APIException If the transmission fails.
	 */
	public void send(EID destination, Integer lifetime, byte[] data, Bundle.Priority priority) throws APIException
	{
		// wrapper to the send(Bundle) function
		Bundle bundle = new Bundle(destination,lifetime);
		bundle.appendBlock(new PayloadBlock(data));
		bundle.setPriority(priority);
		send(bundle);
	}
	
	/**
	 * Send a bundle directly to the daemon. The given object has to encode its own payload into base64 data.
	 * @param destination The destination of the bundle.
	 * @param lifetime The lifetime of the bundle.
	 * @param obj The self-encoding object.
	 * @throws APIException If the transmission fails.
	 */
	public void send(EID destination, Integer lifetime, SelfEncodingObject obj) throws APIException
	{
		send(destination, lifetime, obj, Bundle.Priority.NORMAL);
	}
	
	/**
	 * Send a bundle directly to the daemon. The given object has to encode its own payload into base64 data.
	 * @param destination The destination of the bundle.
	 * @param lifetime The lifetime of the bundle.
	 * @param obj The self-encoding object.
	 * @param priority The priority of the bundle
	 * @throws APIException If the transmission fails.
	 */
	public void send(EID destination, Integer lifetime, SelfEncodingObject obj, Bundle.Priority priority) throws APIException
	{
		// wrapper to the send(Bundle) function
		Bundle bundle = new Bundle(destination,lifetime);
		bundle.appendBlock(new PayloadBlock(new SelfEncodingObjectBlockData(obj)));
		bundle.setPriority(priority);
		send(bundle);
	}
	
	/**
	 * Send a bundle directly to the daemon. The given stream has to provide already base64 encoded data.
	 * @param destination The destination of the bundle.
	 * @param lifetime The lifetime of the bundle.
	 * @param stream The stream containing the encoded payload data.
	 * @param length The length of the un-encoded payload.
	 * @throws APIException If the transmission fails.
	 */
	@Deprecated
	public void sendBase64(EID destination, Integer lifetime, InputStream stream, Long length) throws APIException
	{
		class ImplSelfEncodingObject implements SelfEncodingObject {
			
			private InputStream stream = null;
			private Long length = 0L;
			
			public ImplSelfEncodingObject(InputStream stream, Long length)
			{
				this.stream = stream;
				this.length = length;
			}
			
			@Override
			public Long getLength() {
				return this.length;
			}
			
			@Override
			public void encode64(OutputStream output) throws IOException {
				byte[] buf = new byte[65535];
				Integer len = 0;

				while ( (len = this.stream.read(buf)) > 0 )
				{
					output.write(buf, 0, len);
				}
			}
		};
		
		send(destination, lifetime, new ImplSelfEncodingObject(stream, length));
	}
	
	/**
	 * Send a bundle directly to the daemon. The given stream is used as payload of the bundle.
	 * @param destination The destination of the bundle.
	 * @param lifetime The lifetime of the bundle.
	 * @param stream The stream containing the payload data.
	 * @param length The length of the payload.
	 * @throws APIException If the transmission fails.
	 */
	public void send(EID destination, Integer lifetime, InputStream stream, Long length) throws APIException
	{
		send(destination, lifetime, stream, length, Bundle.Priority.NORMAL);
	}
	
	/**
	 * Send a bundle directly to the daemon. The given stream is used as payload of the bundle.
	 * @param destination The destination of the bundle.
	 * @param lifetime The lifetime of the bundle.
	 * @param stream The stream containing the payload data.
	 * @param length The length of the payload.
	 * @param priority The priority of the bundle
	 * @throws APIException If the transmission fails.
	 */
	public void send(EID destination, Integer lifetime, InputStream stream, Long length, Bundle.Priority priority) throws APIException
	{
		Bundle bundle = new Bundle(destination, lifetime);
		bundle.appendBlock(new PayloadBlock(new InputStreamBlockData(stream, length)));
		bundle.setPriority(priority);
		send(bundle);
	}
	
	/**
	 * Send a bundle directly to the daemon. The given java object gets serialized and the data used as payload. 
	 * @param destination The destination of the bundle.
	 * @param lifetime The lifetime of the bundle.
	 * @param obj The java object to serialize.
	 * @throws APIException If the transmission fails.
	 */
	public void send(EID destination, Integer lifetime, Object obj) throws APIException
	{
		send(destination, lifetime, obj, Bundle.Priority.NORMAL);
	}
	
	/**
	 * Send a bundle directly to the daemon. The given java object gets serialized and the data used as payload. 
	 * @param destination The destination of the bundle.
	 * @param lifetime The lifetime of the bundle.
	 * @param obj The java object to serialize.
	 * @param priority The priority of the bundle
	 * @throws APIException If the transmission fails.
	 */
	public void send(EID destination, Integer lifetime, Object obj, Bundle.Priority priority) throws APIException
	{
		Bundle bundle = new Bundle(destination, lifetime);
		bundle.appendBlock(new PayloadBlock(obj));
		bundle.setPriority(priority);
		send(bundle);
	}
	
	/**
	 * Decodes a object out of the base64 encoded payload data.
	 * @param data The base64 encoded payload data.
	 * @return The decoded object.
	 * @throws APIException If the decode process fails.
	 */
	public Object decodeObject(String data) throws IOException, ClassNotFoundException
	{
		// decode Base64 payload
		byte[] base64_data = null;
			
		base64_data = Base64.decode(data.getBytes());		
		
		// deserialize the byte array to an object
		ObjectInputStream obj;
		
		ByteArrayInputStream ba = new ByteArrayInputStream(base64_data);
		obj = new ObjectInputStream(ba);
		return obj.readObject();
	}
	
	private void startBundle(Integer procflags, EID destination, Integer lifetime, Integer numBlocks) throws APIException
	{
		// upload bundle to the daemon
		send("Destination: " + destination.toString());
		send("Lifetime: " + String.valueOf(lifetime));
		send("Processing flags: " + String.valueOf(procflags));
		send("Blocks: " + String.valueOf(numBlocks));
		send("");
	}
	
	private void startBlock(Integer type, Long length, Boolean lastblock) throws APIException
	{
		send("Block: " + String.valueOf(type));
		
		if (lastblock)
		{
			send("Flags: LAST_BLOCK");
		}

		send("Length: " + String.valueOf(length));
		send("");
	}
	
	/**
	 * Set the application endpoint ID for this client connection.
	 * This EID will be used as source address of outgoing bundles.
	 * @param id The application name to set.
	 * @throws APIException If the request fails.
	 */
	public synchronized void setEndpoint(String id) throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// send endpoint request
		if (query("set endpoint " + id) != 200)
		{
			// error
			throw new APIException("set endpoint failed");
		}
	}
	
	/**
	 * Add a registration to this client connection of the form $nodename/$id.
	 * The registration will not be used as the source when sending bundles,
	 * @see setEndpoint for this purpose
	 * 
	 * @param id the application name that will be concatenated to the daemons EID
	 * @throws APIException If the request fails.
	 */
	public synchronized void addEndpoint(String id) throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");

		// send endpoint request
		if (query("endpoint add " + id) != 200)
		{
			// error
			throw new APIException("set endpoint failed");
		}
	}
	
	public synchronized EID getEndpoint() throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");

		// send endpoint request
		Response resp = queryWithResponse("endpoint get");
		if (resp.getCode() != 200)
		{
			throw new APIException("endpoint get failed");
		}
		
		String[] responseWords = resp.getData().split(" ");
		if(responseWords.length < 3)
		{
			throw new APIException("endpoint get returned not enough data");
		}
		
		return new SingletonEndpoint(responseWords[2]);
	}
	
	/**
	 * Remove a registration of the form $nodename/$id.
	 * @param id the application name that will be concatenated to the daemons EID
	 * @throws APIException If the request fails.
	 */
	public synchronized void removeEndpoint(String id) throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// send endpoint request
		if (query("endpoint del " + id) != 200)
		{
			// error
			throw new APIException("set endpoint failed");
		}
	}
	
	/**
	 * Add a registration to this client connection.
	 * @param eid The EID to register.
	 * @throws APIException If the request fails.
	 */
	public synchronized void addRegistration(GroupEndpoint eid) throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// send endpoint request
		if (query("registration add " + eid.toString()) != 200)
		{
			// error
			throw new APIException("registration add failed");
		}
	}
	
	/**
	 * Remove a registration of this client connection.
	 * @param eid The EID to remove.
	 * @throws APIException If the request fails.
	 */
	public synchronized void removeRegistration(GroupEndpoint eid) throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// send endpoint request
		send("registration del " + eid.toString());
		
		// read answer
		if (_receiver.getResponse().getCode() != 200)
		{
			// error
			throw new APIException("registration removal failed");
		}
	}
	
	/**
	 * Get all neighbors available to the daemon.
	 * @return A list of EIDs. Each of them is an available neighbor.
	 * @throws APIException If the request fails.
	 */
	public synchronized List<String> getNeighbors() throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// query for registration list
		if (query("neighbor list") != 200)
		{
			throw new APIException("neighbor list failed");
		}
		
		return _receiver.getList();
	}
	
	/**
	 * Get all registrations of this connection.
	 * @return A list with EIDs.
	 * @throws APIException If the request fails.
	 */
	public synchronized List<String> getRegistration() throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// query for registration list
		if (query("registration list") != 200)
		{
			throw new APIException("registration get failed");
		}
		
		return _receiver.getList();
	}
	
	/**
	 * Get the EID of the dtn daemon
	 * @return The daemons EID
	 * @throws APIException If the request fails.
	 */
	public synchronized EID getNodeName() throws APIException
	{
		if(remoteEID != null)
			return remoteEID;
		
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		Response resp = queryWithResponse("nodename");
		// query for remote's EID
		if (resp.getCode() != 200)
		{
			throw new APIException("nodename failed");
		}
		
		String[] responseWords = resp.getData().split(" ");
		if(responseWords.length < 2)
		{
			throw new APIException("nodename returned not enough data");
		}
		
		remoteEID = new SingletonEndpoint(responseWords[1]);
		
		return remoteEID;
	}
	
	/**
	 * Load a bundle into the remote register.
	 * @param id The bundle ID of the bundle to load.
	 * @throws APIException If the request fails.
	 */
	public synchronized void loadBundle(BundleID id) throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		send("bundle load " + id.toString());
		
		// read answer
		if (_receiver.getResponse().getCode() != 200)
		{
			// error
			throw new APIException("bundle load failed");
		}
	}
	
	/**
	 * Load the next bundle in the queue into the remote register
	 * and start the API transfer of the bundle.
	 * @throws APIException If the request fails.
	 */
	public synchronized void loadAndGetBundle() throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// load bundle
		int ret = query("bundle load queue");
		
		if (ret == 200)
		{
			// query bundle
			if (query("bundle get") != 200)
			{
				// error
				throw new APIException("bundle get failed");
			}
		}
		else
		{
			if (ret == 400)
			{
				throw new APIException("no bundle available");
			}
			else
			{
				// error
				throw new APIException("bundle load failed");
			}
		}
	}
	
	/**
	 * Load the next bundle in the queue into the remote register.
	 * @throws APIException If the request fails.
	 */
	public synchronized void loadBundle() throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// load bundle
		if (query("bundle load queue") != 200)
		{
			// error
			throw new APIException("bundle load failed");
		}
	}
	
	/**
	 * Start the API transfer of the bundle in the remote register.
	 * @throws APIException If the request fails.
	 */
	public synchronized Bundle getBundle() throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// query bundle
		if (query("bundle get") != 200)
		{
			// error
			throw new APIException("bundle get failed");
		}
		
		return null;
	}
	
	/**
	 * The deletes the bundle in the remote register. This method also deleted
	 * the bundle in the daemon storage.
	 * @throws APIException If the request fails.
	 */
	public synchronized void freeBundle() throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// query bundle
		if (query("bundle free") != 200)
		{
			// error
			throw new APIException("bundle free failed");
		}
	}
	
	/**
	 * Mark the given bundle as delivered. This may delete the bundle in the storage of the daemon.
	 * @throws APIException If the request fails.
	 */
	public synchronized void markDelivered(BundleID bundle) throws APIException
	{
		// throw exception if not connected
		if (state != State.CONNECTED) throw new APIException("not connected");
		
		// query bundle
		if (query("bundle delivered " + bundle.toString() ) != 200)
		{
			// error
			throw new APIException("bundle delivered failed");
		}
	}
	
	/**
	 * Send a command to the daemon.
	 * @param cmd The command to send.
	 * @throws IOException
	 */
	private synchronized void send(String cmd) throws APIException
	{
		try {
			debug("[Send] " + cmd);
			_writer.write(cmd);
			_writer.newLine();
			_writer.flush();
		} catch (IOException e) {
			throw new APIException("send failed: " + cmd);
		}
	}
	
	/**
	 * Send a command to the daemon.
	 * @param cmd The command to send.
	 * @throws IOException
	 */
	private synchronized Integer query(String cmd) throws APIException
	{
		return queryWithResponse(cmd).getCode();
	}
	
	private synchronized Response queryWithResponse(String cmd) throws APIException
	{
		try {
			debug("[Query] " + cmd);
			_writer.write(cmd);
			_writer.newLine();
			_writer.flush();
			
			return _receiver.getResponse();
		} catch (IOException e) {
			throw new APIException("query failed: " + cmd);
		}
	}
	
	private class DataReceiver extends Thread implements SABHandler {

		private BlockingQueue<Response> queue = new SynchronousQueue<Response>();
		private BlockingQueue<List<String> > listqueue = new SynchronousQueue<List<String> >();
		private SABParser p = new SABParser();
		private ExtendedClient client = null;

		public DataReceiver(ExtendedClient client)
		{
			this.client = client;
		}
		
		@Override
		public void run() {
			try {
				p.parse(istream, this);
			} catch (SABException e) {
				e.printStackTrace();
				client.mark_error();
			}
		}
		
		public void abort()
		{
			p.abort();
			queue.offer(new Response(-1, ""));
			debug("abort queued");
		}
		
		/**
		 * Read the response to a command. If a notify is received first, it will 
		 * call the listener to process the notify message.
		 * @return The received return code.
		 * @throws IOException
		 */
		public Response getResponse() throws APIException
		{
			try {
				Response obj = queue.take();
				if (obj.getCode() == -1) throw new APIException("getResponse() failed: response was -1");
				return obj;
			} catch (InterruptedException e) {
				throw new APIException("Interrupted");
			}
		}
		
		public List<String> getList() throws APIException
		{
			try {
				return listqueue.take();
			} catch (InterruptedException e) {
				throw new APIException("Interrupted");
			}
		}

		@Override
		public void startStream() {
			synchronized(handler_mutex) {
				if (handler != null) handler.startStream();
			}
		}

		@Override
		public void endStream() {
			synchronized(handler_mutex) {
				if (handler != null) handler.endStream();
			}
		}

		@Override
		public void startBundle() {
			synchronized(handler_mutex) {
				if (handler != null) handler.startBundle();
			}
		}

		@Override
		public void endBundle() {
			synchronized(handler_mutex) {
				if (handler != null) handler.endBundle();
			}
		}

		@Override
		public void startBlock(Integer type) {
			synchronized(handler_mutex) {
				if (handler != null) handler.startBlock(type);
			}
		}

		@Override
		public void endBlock() {
			synchronized(handler_mutex) {
				if (handler != null) handler.endBlock();
			}
		}

		@Override
		public void attribute(String keyword, String value) {
			synchronized(handler_mutex) {
				if (handler != null) handler.attribute(keyword, value);
			}
		}

		@Override
		public void characters(String data) throws SABException {
			synchronized(handler_mutex) {
				if (handler != null) handler.characters(data);
			}
		}

		@Override
		public void notify(Integer type, String data) {
			synchronized(handler_mutex) {
				if (handler != null) handler.notify(type, data);
			}
		}

		@Override
		public void response(Integer type, String data) {
			debug("[Response] " + String.valueOf(type) + ", " + data);
			try {
				queue.put(new Response(type, data));
			} catch (InterruptedException e) { }
		}

		@Override
		public void response(List<String> data) {
			try {
				listqueue.put(data);
			} catch (InterruptedException e) { }
		}
	};
}
