package ibrdtn.api.object;

import java.io.IOException;
import java.io.OutputStream;

public class ByteArrayBlockData extends Block.Data {
	
	private byte[] _data;

	public ByteArrayBlockData(byte[] data) {
		_data = data;
	}

	@Override
	public void writeTo(OutputStream stream) throws IOException {
		stream.write(_data);
	}

	@Override
	public long size() {
		return _data.length;
	}

}
