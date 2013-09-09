package de.tubs.ibr.dtn.service;

import android.content.Intent;
import de.tubs.ibr.dtn.DaemonState;

public interface DaemonProcessHandler {
    public void onStateChanged(DaemonState state);
    public void onNeighborhoodChanged();
    public void onEvent(Intent intent);
}
