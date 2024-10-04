import java.util.ArrayList;

public class BufferMap {
    private int size;
    private int[] buffer;

    public BufferMap(int size) {
        this.size = size;
        this.buffer = new int[size];
    }

    public int getSize() {
        return size;
    }

    public int get(int index) {
        return buffer[index];
    }

    public void set(int index) {
        buffer[index] = 1;
    }

    public void unset(int index) {
        buffer[index] = 0;
    }

    public boolean isFull() {
        for (int i = 0; i < size; i++) {
            if (buffer[i] == 0) {
                return false;
            }
        }
        return true;
    }

    public boolean isEmpty() {
        for (int i = 0; i < size; i++) {
            if (buffer[i] == 1) {
                return false;
            }
        }
        return true;
    }

    public int getOnes() {
        int c = 0;
        for (int i : buffer) {
            if (i == 1)
                c++;
        }
        return c;
    }

    public ArrayList<BufferMap> slice(int max_ones) {
        ArrayList<BufferMap> bms = new ArrayList<>();

        BufferMap current = new BufferMap(this.getSize());
        int countOnes = 0;

        for (int i = 0; i < this.getSize(); i++) {
            if (this.get(i) == 1) {
                if (countOnes == max_ones) {
                    bms.add(current);
                    current = new BufferMap(this.getSize());
                    countOnes = 0;
                }
                current.set(i);
                countOnes++;
            }
        }

        if (countOnes > 0) {
            bms.add(current);
        }

        return bms;
    }

    public static void setBit(byte[] byteArray, int position, boolean value) {
        int byteIndex = position / 8; // Trouver l'index du byte
        int bitIndex = position % 8; // Trouver l'index du bit dans ce byte
    
        if (value) {
            byteArray[byteIndex] |= (1 << bitIndex); // Mettre le bit à 1
        } else {
            byteArray[byteIndex] &= ~(1 << bitIndex); // Mettre le bit à 0
        }
    }

    public byte[] toByteArray() {
        byte[] byteArray = new byte[(size + 7) / 8];
        for (int i = 0; i < size; i++) {
            setBit(byteArray, i, buffer[i] == 1);
        }
        // System.out.println("Buffermap transformée" + new String(byteArray));
        return byteArray;
    }


    public static BufferMap fromByteArray(byte[] byteArray, int numPieces) {
        BufferMap bm = new BufferMap(numPieces);
        for (int i = 0; i < numPieces; i++) {
            bm.buffer[i] = (byteArray[i / 8] & (1 << (i % 8))) != 0 ? 1 : 0;
        }
        // System.out.println("BUFFERMAP DETRANSFORMEE: " + bm.toString());
        // System.out.println("BUFFERMAP DETRANSFORMEE: " + bm.toString().length() + " " + new String(byteArray).length());
        // System.out.println("BUFFERMAP DETRANSFORMEE: " + new String(byteArray));
        return bm;
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();

        for (int i = 0; i < size; i++) {
            sb.append(buffer[i]);

        }

        return sb.toString();
    }

    public static BufferMap fromString(String s) {
        s = s.trim();
        BufferMap bm = new BufferMap(s.length());
        for (int i = 0; i < s.length(); i++) {

            if (s.charAt(i) == '1') {
                bm.set(i);

            }
        }
        // System.out.println("BufferMap: " + bm.toString() + " from " + s + " length: " + s.length());
        return bm;
    }
}
