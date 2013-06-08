package de.tubs.ibr.dtn.p2p.scheduler;

import java.util.Random;

public class ProbalisticStrategie implements Strategy {

    @Override
    public Slot getNextSlot(String info) {
        int n = new Random().nextInt(2);
        return new Slot(State.values()[n], 10, "" + n);
    }

    @Override
    public State getCurrentState(String info) {
        int n = 0;
        try {
            n = Integer.parseInt(info);
        } catch (Exception e) {
        }
        return State.values()[n];
    }

}
