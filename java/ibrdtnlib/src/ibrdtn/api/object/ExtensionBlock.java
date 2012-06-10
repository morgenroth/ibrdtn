package ibrdtn.api.object;

public class ExtensionBlock extends Block {
	
	private Data _data;

	public ExtensionBlock(int type, Data data) {
		super(type);
		_data = data;
	}

	@Override
	Data getData() {
		return _data;
	}

	public String toString() {
		return "ExtensionBlock: length="+_data.size()
			+",type="+getType();
	}

}
