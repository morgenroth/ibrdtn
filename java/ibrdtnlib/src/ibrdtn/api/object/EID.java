package ibrdtn.api.object;

public abstract class EID implements Comparable<EID> {
	protected String _eid = null;
	
	protected EID(String eid) {
		_eid = eid;
	}
	
	public String toString() {
		return _eid;
	}

	@Override
	public int compareTo(EID o) {
		return _eid.compareTo(o._eid);
	}
}
