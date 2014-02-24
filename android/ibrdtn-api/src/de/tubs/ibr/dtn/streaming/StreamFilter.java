package de.tubs.ibr.dtn.streaming;

import de.tubs.ibr.dtn.api.Bundle;

public interface StreamFilter {
    public boolean onHandleStream(Bundle bundle);
}
