public class HaveMessage {
    private byte[] message;

    public HaveMessage(byte[] head, byte[] key, byte[] bm){
        String msg = "have " + new String(key) + " " + new String(bm) + "\n";
        this.message = msg.getBytes();
    }

    public byte[] getBytes() {
        return message;
    }
}
