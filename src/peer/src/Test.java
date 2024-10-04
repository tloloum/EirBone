public class Test {
    public static void main(String[] args) {
        System.out.println("Tests start");

        // Test peer
        Peer peer = new Peer(2222, "localhost", 8000, "owned");
        Thread peerThread = new Thread(peer);
        peerThread.start();

        Peer peer2 = new Peer(3333, "localhost", 8000, "owned");
        Thread peerThread2 = new Thread(peer2);
        peerThread2.start();

        // Test tracker communication

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        System.out.println("Tracker communication test");
        System.out.println("look filename=file1.txt");
        System.out.println(peer.look("file1.txt"));
        System.out.println("look *");
        System.out.println(peer.look("*"));
        System.out.println("getfile");
        String key = peer.look_first("logobar.jpg").getKey();
        String key2 = peer2.look_first("taiste.txt").getKey();
        String[] files = peer.getfile(key);
        for (String file : files) {
            System.out.println(file);
        }

        // Test p2p communication
        System.out.println("P2P communication test");
        System.out.println("connect");
        int client1;
        try {
            client1 = peer.connect("localhost", 3333);
        } catch (Exception e) {
            client1 = -1;
        }
        System.out.println("interested");
        //BufferMap buffermap = peer.interested(key, client1);
        //BufferMap buffermap2 = peer.interested(key2, client1);
        System.out.println("respond_to_interested");

        System.out.println("getpieces");

        System.out.println(new String(new GetpiecesMessage("keyyy", BufferMap.fromString("1001100101")).getBytes()));

        // peer.getpieces(key, BufferMap.fromString("10000000000"), client1);

        System.out.println("Test download");

        ManagedFile logo = peer.look_key(key);
        ManagedFile file = peer2.look_key(key2);

        // peer.download(key2, "test2.txt", file.getLength(), file.getPieceLength());
        // peer.download(key, "test.jpg", logo.getLength(), logo.getPieceLength());

    }
}
