package ibrdtn.api.object;

import ibrdtn.api.NullOutputStream;

import java.io.IOException;
import java.io.ObjectOutputStream;
import java.io.OutputStream;

/**
 * This class wraps an object to be used as block data.
 * The object will be serialized when written to a stream.
 */
public class ObjectBlockData extends Block.Data {
	
	Object _obj;
	
	public ObjectBlockData(Object obj) {
		_obj = obj;
	}

	@Override
	public long size() {
		NullOutputStream nullStream = new NullOutputStream();
		try {
			writeTo(nullStream);
			nullStream.flush();
			return nullStream.getCount();
		} catch (IOException e) {
			return 0;
		}
	}

	@Override
	public void writeTo(OutputStream stream) throws IOException {
		ObjectOutputStream oos = new ObjectOutputStream(stream);
		oos.writeObject(_obj);
	}

}
