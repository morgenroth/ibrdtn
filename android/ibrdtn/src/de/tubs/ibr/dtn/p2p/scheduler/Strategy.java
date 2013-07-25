package de.tubs.ibr.dtn.p2p.scheduler;

public interface Strategy {

	Slot getNextSlot(String info);

	State getCurrentState(String info);

}
