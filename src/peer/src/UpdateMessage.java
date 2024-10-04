public class UpdateMessage implements Message{
    private byte[] message;

    public UpdateMessage(int port, FileLibrary fl) {
        String msg = "update ";
        String seed = fl.announce();

        if (seed != null) {
            msg += "seed " + seed + "\n";
        }
        // if (leech != null) {
        //     msg += "leech " + leech + "\n";
        // }

        this.message = msg.getBytes();
        SimpleLogger.debug("Announce message: " + msg);
    }

    public UpdateMessage(byte[] message) {
        this.message = message;
    }

    public byte[] getBytes() {
        return message;
    }
}
