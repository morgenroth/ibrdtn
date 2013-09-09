package de.tubs.ibr.dtn.p2p.scheduler;

public class Slot {
    private State s;
    private int duration;
    private String info;

    public Slot(State s, int duration, String info) {
        super();
        this.s = s;
        this.duration = duration;
        this.info = info;
    }

    public State getS() {
        return s;
    }

    public void setS(State s) {
        this.s = s;
    }

    public int getDuration() {
        return duration;
    }

    public void setDuration(int duration) {
        this.duration = duration;
    }

    public String getInfo() {
        return info;
    }

    public void setInfo(String info) {
        this.info = info;
    }
}
