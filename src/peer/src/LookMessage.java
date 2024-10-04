public class LookMessage implements Message {
    private byte[] message;

    public LookMessage(String crit) {
        if (crit.equals("*")) {
            this.message = "look *\n".getBytes();
        } else {
            String msg = "look [" + crit + "]\n";
            this.message = msg.getBytes();
        }
        SimpleLogger.debug("Look message: " + new String(this.message));
    }

    public byte[] getBytes() {
        return message;
    }

    public static FileLibrary lookResponse(Message response) {
        String r = new String(response.getBytes());
        r = r.substring(6, r.length()).replace("]", "");
        String[] files = r.split(" ");
        SimpleLogger.info("Look response " + r);
        FileLibrary fl = new FileLibrary();
        for (int i = 0; i < files.length -1; i+=4) {
            fl.addFile(new ManagedFile(files[i], Integer.parseInt(files[i+1]), Integer.parseInt(files[i+2]), files[i+3], false));
        }
        return fl;
    }
}
