package ibrdtn.api.object;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

class SDNVOutputStream extends OutputStream {

	private Queue<SDNV> _queue = new LinkedBlockingQueue<SDNV>();
	private ByteBuffer _currentSDNV = ByteBuffer.allocate(SDNV.MAX_SDNV_BYTES);
	private int _currentIndex = 0;

	private boolean _failed = false;

	public SDNV nextSDNV() throws IOException {
		if(_queue.isEmpty())
			throw new IOException("No SDNV in queue");
		return _queue.remove();
	}

	@Override
	public void write(int b) throws IOException {
		if(_failed)
			throw new IOException("SDNVOutputStream in failed state");
		_currentSDNV.put((byte)b);
		++_currentIndex;

		if(((byte) b & (byte) 0x80) == 0) {
			//last byte, create SDNV
			try {
				_queue.add(new SDNV(_currentSDNV.array()));
				_currentSDNV.clear();
				_currentIndex = 0;
			} catch (NumberFormatException ex) {
				_failed = true;
				throw new IOException(ex.getMessage());
			}
		}
		else if(_currentIndex >= _currentSDNV.capacity()) {
			//SDNV too long, set failed state and throw exception
			_failed = true;
			throw new IOException("SDNVs only supported up to " + _currentSDNV.capacity() + "Bytes.");
		}
	}
}
