package de.tubs.ibr.dtn.streaming;

public enum MediaType {
    BINARY(0),
    MEDIA_AUDIO(1),
    MEDIA_VIDEO(2),
    MEDIA_MIXED(3);
    
    public int code = 0;
    
    private MediaType(int i) {
        code = i;
    }
    
    public static MediaType valueOf(int i) {
        switch (i) {
            default:
                return BINARY;
            case 1:
                return MEDIA_AUDIO;
            case 2:
                return MEDIA_VIDEO;
            case 3:
                return MEDIA_MIXED;
        }
    }
}
