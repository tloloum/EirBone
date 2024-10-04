import java.util.BitSet;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Utils {

    private Utils() {
    }

    public static String BitSetToStringHard(BitSet bitSet) {
        byte[] bytes = bitSet.toByteArray();
        StringBuilder asciiString = new StringBuilder();
        for (byte b : bytes) {
            asciiString.append((char) b);
        }

        return asciiString.toString();
    }

    public static String extractDataBetweenBrackets(String input) {
        Pattern pattern = Pattern.compile("\\[(.*?)\\]");
        Matcher matcher = pattern.matcher(input);
        if (matcher.find()) {
            return matcher.group(1); // Group 1 contains the data between brackets
        } else {
            return null; // No data found between brackets
        }
    }

    public static void printBits(byte[] bytes) {
        for (byte b : bytes) {
            for (int i = 0; i < 8; i++) {
                System.out.print((b & (1 << i)) == 0 ? "0" : "1");
            }
        }
    }

    public static byte[] concat_byte_array(byte[] a, byte[] b) {
        byte[] c = new byte[a.length + b.length];
        System.arraycopy(a, 0, c, 0, a.length);
        System.arraycopy(b, 0, c, a.length, b.length);
        return c;
    }

    //Some test code
    public static void main(String[] args) {


        //1100010

        BitSet bitSet = new BitSet();
        bitSet.set(0);
        bitSet.set(5);

        System.out.println(BitSetToStringHard(bitSet));
    }

}
