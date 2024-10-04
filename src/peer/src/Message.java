public interface Message {
    public byte[] getBytes();

    public static Message fromBytes(byte[] bytes) {
        return new AnnounceMessage(bytes);
    }

    public static String getCommand(byte[] bytes) {
        String message = new String(bytes);
        String[] parts = message.split(" ", 2);
        return parts[0];
    }

    public static String getArgument(byte[] bytes) {
        String message = new String(bytes);
        String[] parts = message.split(" ", 2);
        return parts[1];
    }
}
