package de.tubs.ibr.dtn.streaming;

public enum MediaType {
    UNKNOWN(0),
    BINARY(1),
    MEDIA_AUDIO(2),
    MEDIA_VIDEO(3),
    MEDIA_MIXED(4);
    
    public int code = 0;
    
    private MediaType(int i) {
        code = i;
    }
    
    public static MediaType valueOf(int i) {
        switch (i) {
            default:
                return UNKNOWN;
            case 1:
                return BINARY;
            case 2:
                return MEDIA_AUDIO;
            case 3:
                return MEDIA_VIDEO;
            case 4:
                return MEDIA_MIXED;
        }
    }
}
