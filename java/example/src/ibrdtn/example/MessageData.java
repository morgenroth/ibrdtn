package ibrdtn.example;

import java.io.Serializable;

/**
 * A custom payload data format.
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class MessageData implements Serializable {

    private String id;
    private String correlationId;

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getCorrelationId() {
        return correlationId;
    }

    public void setCorrelationId(String correlationId) {
        this.correlationId = correlationId;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("\tID:").append(getId()).append("\n");
        sb.append("\tCorrelation:").append(getCorrelationId()).append("\n");
        return sb.toString();
    }
}
