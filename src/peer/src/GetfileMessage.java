public class GetfileMessage implements Message{
    private byte[] message;

    public GetfileMessage(String key) {
        this.message = ("getfile " + key + "\r\n").getBytes();
        SimpleLogger.debug("Getfile message: " + new String(this.message));
    }

    public GetfileMessage(ManagedFile file) {
        this.message = ("getfile " + file.getKey() + "\r\n").getBytes();
        SimpleLogger.debug("Getfile message: " + new String(this.message));
    }

    public static String[] getfileResponse(Message response) {
        String r = new String(response.getBytes());
        if (!r.contains("peers")) return null;
        r = r.split(" ", 3)[2].replace("[", "").replace("]", "");
        SimpleLogger.debug("Getfile response: " + r);
        String[] ipports = r.split(" ");
        return ipports;
    }

    public byte[] getBytes() {
        return message;
    }
}
