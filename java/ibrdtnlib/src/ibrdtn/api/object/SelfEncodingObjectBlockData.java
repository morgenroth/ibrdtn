package ibrdtn.api.object;

import java.io.IOException;
import java.io.OutputStream;

public class SelfEncodingObjectBlockData extends Block.Data {

	SelfEncodingObject _object;
	
	public SelfEncodingObjectBlockData(SelfEncodingObject object) {
		_object = object;
	}
	
	@Override
	public long size() {
		return _object.getLength();
	}

	@Override
	public void writeTo(OutputStream stream) throws IOException {
		_object.encode64(stream);
	}

}
