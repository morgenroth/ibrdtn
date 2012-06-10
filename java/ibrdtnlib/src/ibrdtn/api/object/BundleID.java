package ibrdtn.api.object;

public class BundleID {
	
	private String source = null;
	private Long timestamp = null;
	private Long sequencenumber = null;

	public BundleID()
	{
	}
	
	public BundleID(String source, Long timestamp, Long sequencenumber)
	{
		this.source = source;
		this.timestamp = timestamp;
		this.sequencenumber = sequencenumber;
	}

	public String getSource() {
		return source;
	}

	public void setSource(String source) {
		this.source = source;
	}

	@Override
	public String toString() {
		return String.valueOf(this.timestamp) + " " + String.valueOf(this.sequencenumber) + " " + this.source;
	}

	public Long getTimestamp() {
		return timestamp;
	}

	public void setTimestamp(Long timestamp) {
		this.timestamp = timestamp;
	}

	public Long getSequencenumber() {
		return sequencenumber;
	}

	public void setSequencenumber(Long sequencenumber) {
		this.sequencenumber = sequencenumber;
	}
}
