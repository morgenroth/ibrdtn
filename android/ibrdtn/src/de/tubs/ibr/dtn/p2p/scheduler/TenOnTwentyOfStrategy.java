package de.tubs.ibr.dtn.p2p.scheduler;

public class TenOnTwentyOfStrategy implements Strategy {

    @Override
    public Slot getNextSlot(String info) {
        Slot s;
        if ("on".equals(info)) {
            s = new Slot(State.OFF, 10, "off");
        } else {
            s = new Slot(State.ON, 20, "on");
        }
        return s;
    }

    @Override
    public State getCurrentState(String info) {
        State s = State.OFF;
        if ("on".equals(info)) {
            s = State.ON;
        }
        return s;
    }

}
