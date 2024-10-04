import java.util.ArrayList;
import java.util.HashMap;
public class GetpiecesMessage implements Message {
    private byte[] message;

    public GetpiecesMessage(String key, BufferMap bm) {
        String e = "";
        for (int index = 0; index < bm.getSize(); index++) {
            SimpleLogger.debug("Index: " + index);
            if (bm.get(index) == 1) {
                e += index + " ";
            }
        }
        this.message = ("getpieces " + key + " " + "[" + e + "]" + "\n").getBytes();
        SimpleLogger.debug("Getpieces message: " + new String(this.message));
    }

    public GetpiecesMessage(ManagedFile file) {
        this.message = ("getpieces " + file.getKey() + "\n").getBytes();
        SimpleLogger.debug("Getpieces message: " + new String(this.message));
    }

    public byte[] getBytes() {
        return message;
    }

    public static HashMap<Integer, byte[]> getpiecesResponse(Message response, BufferMap asked_pieces, int piece_size) {
        HashMap<Integer, byte[]> pieces = new HashMap<>();
        ArrayList<Integer> asked = new ArrayList<>();

        // Extract pieces to ask
        for (int i = 0; i < asked_pieces.getSize(); i++) {
            if (asked_pieces.get(i) == 1) {
                asked.add(i);
            }
        }
        SimpleLogger.debug("Asked pieces: " + asked);
        byte[] res = response.getBytes();
        try {
            byte[] header = new byte[4];
            System.arraycopy(res, 0, header, 0, 4);
            byte[] filekey = new byte[32];
            System.arraycopy(res, 5, filekey, 0, 32);
            int readHead = 39;
            for (int idx: asked) {
              int lenIdx = numNum(idx);
              byte[] readIdx = new byte[lenIdx];
              System.arraycopy(res, readHead, readIdx, 0, lenIdx);
              readHead += lenIdx + 1;
              if (idx != Integer.parseInt(new String(readIdx))) {
                SimpleLogger.error("Error parsing data");
                return null;
              }
              SimpleLogger.debug("Reading piece " + idx);
              int piece_len;
              if (idx != asked_pieces.getSize() - 1){
                piece_len = piece_size;
              } else {
                SimpleLogger.debug("DERNIER FICHIER");
                piece_len = res.length - readHead -1;
              }
              byte[] piece = new byte[piece_len];
              SimpleLogger.debug("PIECE LENGTH: " + piece_len + " " + readHead + " " + res.length);
              System.arraycopy(res, readHead, piece, 0, piece_len);
              readHead += piece_len;
              readHead += 1; //space after piece
              pieces.put(idx, piece);
            }
           return pieces;
        } catch (Exception e) {
            SimpleLogger.error("Error reading pieces: " + e.getMessage());
            e.printStackTrace();
            return null;
        }
    }

    // Helper function to determine the number of digits in an index
    private static int numNum(int number) {
        if (number == 0)
            return 1;
        return String.valueOf(number).length();
    }
}
