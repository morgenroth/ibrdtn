package ibr.timpner.dtn;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class ReportRegExTest {

    public enum ReasonCode {

        NO_ADDITIONAL_INFORMATION,
        LIFETIME_EXPIRED,
        UNIDIRECTIONAL_FORWARD,
        TRANSMISSION_CANCELED,
        DEPLETED_STORAGE,
        DESTINATION_UNINTELLIGIBLE,
        NO_KNOWN_ROUTE,
        NO_TIMELY_CONTACT,
        BLOCK_UNINTELLIGIBLE
    }

    public static void main(String[] args) {
        String start = "(603\\sNOTIFY\\sREPORT)";
        String url = "(\\bdtn://[-a-zA-Z0-9+&@#/%?=~_|!:,.;]*[-a-zA-Z0-9+&@#/%=~_|])";
        String fragments = "(\\.\\d+:\\d+)?";
        String notificationType = "(\\w+)\\[(\\d+)\\.(\\d+)\\]";

        // 603 NOTIFY REPORT <timestamp>.<seq_nr>[.<frag_offset>:<frag_len>] <src_eid> <reason_code> <type>\\[<timestamp>.<tmp_nanos>\\]
        final Pattern pattern = Pattern.compile(start + "\\s(\\d+)\\.(\\d+)" + fragments + "\\s" + url + "\\s(\\d+)\\s" + notificationType
                + "(.*)");
        final Matcher matcher = pattern.
                matcher("603 NOTIFY REPORT 414503259.0.666:777 dtn://timpner-lx/test-1 0 RECEIPT[1361188059.760648000] ");
        matcher.find();
        System.out.println("Status: " + matcher.group(1));
        System.out.println("Timestamp: " + matcher.group(2));
        System.out.println("Seq: " + matcher.group(3));
        System.out.println("[Frag]: " + matcher.group(4));
        String frag = matcher.group(4);
        if (frag != null) {
            String[] split = frag.split("\\.|:");
            System.out.println("Offset: " + split[1]);
            System.out.println("Length: " + split[2]);
        }

        System.out.println("src: " + matcher.group(5));
        int reasonCode = Integer.parseInt(matcher.group(6));
        System.out.println("Reason code: " + reasonCode + ":" + ReasonCode.values()[reasonCode]);
        System.out.println("Type: " + matcher.group(7));
        System.out.println("Timestamp: " + matcher.group(8));
        System.out.println("Nanos: " + matcher.group(9));
    }
}
