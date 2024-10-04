import java.net.ServerSocket;

public class ConnectionsHandler implements Runnable{
    private Peer peer;
    private ServerSocket serverSocket;
    private int max_msg_size;

    public ConnectionsHandler(int port, Peer p, int max_msg_size) {
        this.peer = p;
        try {
            this.serverSocket = new ServerSocket(port);
            this.max_msg_size = max_msg_size;
        } catch (Exception e) {
            e.printStackTrace();

        }
    }

    @Override
    public void run() {
        while (true) {
            try {
                PeerServer s = new PeerServer(serverSocket.accept(), peer, max_msg_size);
                Thread t = new Thread(s);
                t.start();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
