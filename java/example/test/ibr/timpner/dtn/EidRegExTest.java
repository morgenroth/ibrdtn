package ibr.timpner.dtn;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class EidRegExTest {

    public static void main(String[] args) {
        // dtn://sbase.dtn
        String data = "ipn:1337 20#discovered#TCP#ip=::ffff:134.169.35.168;port=4556";

        String[] split = data.split("\\s");

        String url = "("
                + "(\\bdtn://[-a-zA-Z0-9+&@#/%?=~_|!:,.;]*[-a-zA-Z0-9+&@#/%=~_|])" // dtn://
                + "|"
                + "(\\bipn:[0-9]*)" // ipn:
                + ")";

        final Pattern eidPattern = Pattern.compile(url);
        final Matcher eidMatcher = eidPattern.matcher(split[0]);
        if (!eidMatcher.matches()) {
            System.err.println("Unexpected eid format: " + split[0]);
        } else {
            System.out.println("OK: " + split[0]);
        }
    }
}
