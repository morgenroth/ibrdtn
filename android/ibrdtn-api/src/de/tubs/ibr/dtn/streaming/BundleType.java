package de.tubs.ibr.dtn.streaming;

public enum BundleType {
    INITIAL(0),
    DATA(1),
    FIN(2);
    
    public int code = 0;
    
    private BundleType(int i) {
        code = i;
    }
    
    public static BundleType valueOf(int i) {
        switch (i) {
            default:
                return INITIAL;
            case 1:
                return DATA;
            case 2:
                return FIN;
        }
    }
}
