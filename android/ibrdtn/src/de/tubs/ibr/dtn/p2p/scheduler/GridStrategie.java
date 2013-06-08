package de.tubs.ibr.dtn.p2p.scheduler;

public class GridStrategie implements Strategy {

    private static final int GRID[][] = { { 0, 1, 0, 0, 0 }, { 0, 1, 0, 0, 0 },
            { 0, 1, 0, 0, 0 }, { 1, 1, 1, 1, 1 }, { 0, 1, 0, 0, 0 } };

    @Override
    public Slot getNextSlot(String info) {
        int y, x;
        String[] splitted;
        try {
            splitted = info.split(":");
            x = Integer.parseInt(splitted[0]);
            y = Integer.parseInt(splitted[1]);
        } catch (Exception e) {
            x = 0;
            y = 0;
        }
        x++;
        if (x >= 5) {
            x = 0;
            y++;
        }
        if (y >= 5) {
            y = 0;
        }
        if (GRID[x][y] == 1) {
            return new Slot(State.ON, 10, x + ":" + y);
        } else {
            return new Slot(State.OFF, 10, x + ":" + y);
        }
    }

    @Override
    public State getCurrentState(String info) {
        int y, x;
        String[] splitted;
        try {
            splitted = info.split(":");
            x = Integer.parseInt(splitted[0]);
            y = Integer.parseInt(splitted[1]);
        } catch (Exception e) {
            x = 0;
            y = 0;
        }
        if (GRID[x][y] == 1) {
            return State.ON;
        } else {
            return State.OFF;
        }
    }
}
