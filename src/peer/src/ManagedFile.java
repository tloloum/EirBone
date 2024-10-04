import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;

public class ManagedFile {
    private String filename;
    private int length;
    private int pieceLength;
    private String key;
    private int numPieces;
    private BufferMap bufferMap;
    private File file;

    // New file for download
    public ManagedFile(String filename, int length, int pieceLength, String key, boolean for_download) {
        this.filename = filename;
        this.length = length;
        ConfigLoader config = new ConfigLoader();
        this.pieceLength = config.getPieceSize();
        this.key = key;
        this.numPieces = (int) Math.ceil((double) length / pieceLength);
        this.bufferMap = new BufferMap(numPieces);
        if (for_download) {
            this.file = new File(config.getDownloadDir()+ "/" + filename);
            try {
                file.createNewFile();
                file.setWritable(true);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        for (int i = 0; i < numPieces; i++) {
            bufferMap.unset(i);
        }
    }

    // Real file
    public ManagedFile(String filepath) {
        File file = new File(filepath);
        this.file = file;
        this.filename = file.getName();
        ConfigLoader config = new ConfigLoader();
        this.pieceLength = config.getPieceSize();
        this.length = (int) file.length();
        this.key = HashUtil.getMD5Checksum(file);
        this.numPieces = (int) Math.ceil((double) length / pieceLength);
        this.bufferMap = new BufferMap(numPieces);
        for (int i = 0; i < numPieces; i++) {
            bufferMap.set(i);
        }
    }

    public String announce() {
        return filename + " " + length + " " + pieceLength + " " + key;
    }

    public String getKey() {
        return key;
    }

    public String getName() {
        return filename;
    }

    public BufferMap getBufferMap() {
        return bufferMap;
    }

    public int getNumPieces(){
        return numPieces;
    }

    public byte[] get_piece(int piece_id) {
      int len;
      if (piece_id == numPieces - 1) {
        len = length % pieceLength; 
      } else {
        len = pieceLength;
      }
      byte[] piece = new byte[len];
      try {
        RandomAccessFile is = new RandomAccessFile(file, "r");
        is.seek(piece_id * pieceLength);
        is.read(piece);
        is.close();
      } catch (Exception e) {
        e.printStackTrace();
      }
      return piece;
    }

  public void set_piece(int piece_id, byte[] piece) {
        try {
            RandomAccessFile raf = new RandomAccessFile(file, "rw");
            raf.seek(piece_id * pieceLength);
            raf.write(piece);
            raf.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        bufferMap.set(piece_id);
    }

    public int getLength() {
        return length;
    }

    public int getPieceLength() {
        return pieceLength;
    }

    public int getLastPieceLength(){
        return length % pieceLength;
    }

    @Override
    public String toString() {
        // Header with ASCII Art
        String header = "\n  ___________ \n< FILE INFO >\n  ----------- \n";
        String underline = "------------------------------------------------\n";

        StringBuilder sb = new StringBuilder();
        sb.append(header);

        // Colorful display of attributes
        sb.append(String.format(" \033[1;36mManagedFilename\033[0m: \033[0;33m%s\033[0m\n", filename));
        sb.append(String.format(" \033[1;36mLength\033[0m: \033[0;33m%d\033[0m bytes\n", length));
        sb.append(String.format(" \033[1;36mPiece Size\033[0m: \033[0;33m%d\033[0m bytes\n", pieceLength));
        sb.append(String.format(" \033[1;36mKey\033[0m: \033[0;33m%s\033[0m\n", key));
        sb.append(underline);

        // BufferMap "status bar" display
        sb.append(" \033[1;36mBufferMap\033[0m: ");
        for (int i = 0; i < Math.min(numPieces, 50); i++) {
            if (bufferMap.get(i) == 1) {
                sb.append("\033[42m \033[0m"); // Green square for "up" bit
            } else {
                sb.append("\033[41m \033[0m"); // Red square for "missing" bit
            }
        }
        sb.append("\n");
        sb.append(String.format(" \033[1;36mNumber of Pieces\033[0m: \033[0;33m%d\033[0m\n", numPieces));
        sb.append(underline);
        return sb.toString();
    }
}
