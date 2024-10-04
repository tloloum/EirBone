import java.net.*;
import java.util.Arrays;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
public class PeerServer implements Runnable {
    private Peer peer;
    private Socket socket;
    private int id;
    private int max_msg_size;
    private Queue<byte[]> messageQueue = new ConcurrentLinkedQueue<>();

    public PeerServer(Socket s, Peer p, int max_msg_size) {
        this.peer = p;
        this.max_msg_size = max_msg_size;
        try {
            this.socket = s;
        } catch (Exception e) {
            SimpleLogger.error("Error creating peer server");
            e.printStackTrace();
        }
        peer.addPeerServer(this);
        this.id = peer.getServerCount();
    }

    public void shutdown() {
        try {
            socket.close();
        } catch (Exception e) {
            SimpleLogger.error("Error closing socket");
            e.printStackTrace();
        }
        peer.removePeerServer(this);
        return;
    }

    @Override
    public void run() {
        Thread messageProcessor = new Thread(this::processMessages);
        messageProcessor.start();
        while (true) {
            try {
                byte[] request = new byte[max_msg_size];
                int bytesRead = socket.getInputStream().read(request);

                if (bytesRead != -1) {
                    SimpleLogger.debug("Adding msg to queue ");
                    messageQueue.offer(Arrays.copyOf(request, bytesRead));
                }
            } catch (Exception e) {
                SimpleLogger.error("Error reading from socket");
                e.printStackTrace();
                shutdown();
                return;
            }
        }
    }

    private void processMessages() {
        SimpleLogger.debug("Starting message processor for peer " + id);
        while (!Thread.currentThread().isInterrupted()) {
            byte[] request = messageQueue.poll(); // Retrieves and removes the head of this queue
            if (request != null) {
                SimpleLogger.debug("Processing msg from queue " + new String(request));
                processRequest(request);
            }
        }
    }

    private void processRequest(byte[] request) {
        SimpleLogger.debug("Distant peer: " + id + " < " + new String(request));
        try {
            String cmd = Message.getCommand(request);
            String response;
            switch (cmd) {
                case "interested":
                    handleInterested(request);
                    response = "\n";
                    break;
                case "have":
                    response = "not implemented";
                    break;
                case "getpieces":
                    handleGetpieces(request);
                    response = "\n";
                    break;

                default:
                    response = "Unknown command";
                    break;
            }
            socket.getOutputStream().write(response.getBytes());
            socket.getOutputStream().flush();
        } catch (Exception e) {
            SimpleLogger.error("Error processing request: " + new String(request));
            e.printStackTrace();
            shutdown();
            return;
        }
    }

    private void handleGetpieces(byte[] request) throws Exception {
        String filekey = new String(request).split(" ", 3)[1];
        String pieces_str = new String(request).split(" ", 3)[2];
        SimpleLogger.debug("pieces: " + pieces_str);
        pieces_str = pieces_str.replace("[", "").replace("]", "");
        SimpleLogger.debug("pieces: " + pieces_str);
        String[] pieces = pieces_str.trim().split(" ");
        int[] id_pieces = new int[pieces.length];
        for (int i = 0; i < pieces.length; i++) {
            try {
                id_pieces[i] = Integer.parseInt(pieces[i]);
            } catch (NumberFormatException e) {
                SimpleLogger.error("Error parsing piece id: " + pieces[i]);
                return;
            }
        }
        byte[] data = peer.respond_to_getpieces(filekey, id_pieces);
        socket.getOutputStream().write(data);
        socket.getOutputStream().flush();
    }

    private void handleInterested(byte[] request) throws Exception {
        byte[] readkey = new byte[32];
        System.arraycopy(request, 11, readkey, 0, 32);
        SimpleLogger.debug("Je vais répondre: " + new String(readkey));
        byte[] have = peer.respond_to_interested(new String(readkey));
        SimpleLogger.debug("Je vais répondre: " + new String(have));
        socket.getOutputStream().write(have);
        socket.getOutputStream().flush();
        SimpleLogger.debug("Je viens de répondre: " + new String(have));
    }
}