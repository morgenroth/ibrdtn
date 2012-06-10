package ibrdtn.api.object;

public class PayloadBlock extends Block {
	
	public static final int type = 1;
	private Data _data;

	public PayloadBlock(Data data) {
		super(type);
		_data = data;
	}
	
	public PayloadBlock(Object obj) {
		super(type);
		_data = new ObjectBlockData(obj);
	}
	
	public PayloadBlock(byte[] data) {
		super(type);
		_data = new ByteArrayBlockData(data);
	}

	@Override
	Data getData() {
		return _data;
	}

	public String toString() {
		return "PayloadBlock: length="+_data.size();
	}

}
