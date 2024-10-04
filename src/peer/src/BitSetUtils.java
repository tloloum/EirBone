import java.util.BitSet;

public class BitSetUtils {
    public static String BitSettoString(BitSet bitSet) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < bitSet.length(); i++) {
            sb.append(bitSet.get(i) ? 1 : 0);
        }
        return sb.toString();
    }

    public static BitSet stringToBitSet(String s) {
        BitSet bitSet = new BitSet(s.length());
        for (int i = 0; i < s.length(); i++) {
            bitSet.set(i, s.charAt(i) == '1');
        }
        return bitSet;
    }
}
