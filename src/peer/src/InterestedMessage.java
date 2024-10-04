public class InterestedMessage implements Message {
    private byte[] message;

    public InterestedMessage(String key) {
        this.message = ("interested " + key + "\n").getBytes();
        SimpleLogger.debug("Interested message: " + new String(this.message));
    }

    public InterestedMessage(ManagedFile file) {
        this.message = ("interested " + file.getKey() + "\n").getBytes();
        SimpleLogger.debug("Interested message: " + new String(this.message));
    }

    public static BufferMap interestedResponse(Message response, int numPieces) {
        byte[] res = response.getBytes();
        SimpleLogger.debug("Interested response: " + new String(res));
        // Peer.printBits(res);
        byte[] dataHeader = new byte[4];
        byte[] fileKey = new byte[32];
        byte[] bufferMap = new byte[numPieces / 8 + 1];
        try {
            System.arraycopy(res, 0, dataHeader, 0, 4);
            System.arraycopy(res, 5, fileKey, 0, 32);
            System.arraycopy(res, 38, bufferMap, 0, numPieces / 8 + 1);
            // Peer.printBits(dataHeader);
            // System.out.println("\n");
            // Peer.printBits(fileKey);
            // System.out.println("\n");
            // Peer.printBits(bufferMap);
            SimpleLogger.debug("File key: " + new String(fileKey) + " " + fileKey.length);

        } catch (Exception e) {
            e.printStackTrace();
        }
        return BufferMap.fromByteArray(bufferMap, numPieces);
    }

    public byte[] getBytes() {
        return message;
    }
}