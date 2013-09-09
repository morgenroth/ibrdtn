package de.tubs.ibr.dtn.p2p.scheduler;

public class StategyAlwaysOn implements Strategy {

	@Override
	public Slot getNextSlot( String info) {
		Slot slot = new Slot(State.ON, 60, "");
		return slot;
	}

	@Override
	public State getCurrentState(String info) {
		return State.ON;
	}

}
