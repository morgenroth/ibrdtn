package de.tubs.ibr.dtn.keyexchange;

public class ProtocolContainer {
	private EnumProtocol protocol;
	private boolean used;
	
	public ProtocolContainer(EnumProtocol protocol, boolean used) {
		this.protocol = protocol;
		this.used = used;
	}

	public EnumProtocol getProtocol() {
		return protocol;
	}

	public void setProtocol(EnumProtocol protocol) {
		this.protocol = protocol;
	}

	public boolean isUsed() {
		return used;
	}

	public void setUsed(boolean used) {
		this.used = used;
	}
}
