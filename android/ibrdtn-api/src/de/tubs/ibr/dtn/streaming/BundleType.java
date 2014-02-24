package de.tubs.ibr.dtn.streaming;

public enum BundleType {
    UNKOWN(0),
    INITIAL(1),
    DATA(2),
    FIN(3);
    
    public int code = 0;
    
    private BundleType(int i) {
        code = i;
    }
    
    public static BundleType valueOf(int i) {
        switch (i) {
            default:
                return UNKOWN;
            case 1:
                return INITIAL;
            case 2:
                return DATA;
            case 3:
                return FIN;
        }
    }
}
