public class AnnounceMessage implements Message {
    private byte[] message;

    public AnnounceMessage(int port, FileLibrary fl){
        String msg = "announce listen " + port + " ";
        String seed = fl.announce();

        if (seed != null)
            msg += "seed " + seed + "\n";

        this.message = msg.getBytes();
    }

    public AnnounceMessage(byte[] message){
        this.message = message;
    }

    public byte[] getBytes(){
        return message;
    }
}
