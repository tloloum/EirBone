import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Queue;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

public class PeerClient implements Client, Runnable {
    private Socket socket;
    private Peer peer;
    private int id;
    private ConcurrentHashMap<String, CompletableFuture<Message>> responseHandlers = new ConcurrentHashMap<>();
    private int msg_max_size;
    private Queue<byte[]> messageQueue = new ConcurrentLinkedQueue<>();

    public PeerClient(String ip, int port, Peer p, int msg_max) throws Exception {
        this.peer = p;
        this.socket = new Socket(ip, port);
        peer.addPeerClient(this);
        this.id = peer.getClientCount() - 1;
        this.msg_max_size = msg_max;
    }

    public void shutdown() {
        try {
            if (!socket.isClosed()) {
                socket.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            peer.removePeerClient(this);
        }
    }

    public int getId() {
        return id;
    }

    public String getIp() {
        return socket.getInetAddress().getHostAddress();
    }

    public int getPort() {
        return socket.getPort();
    }

    public Message send(Message message) {
        try {
            socket.getOutputStream().write(message.getBytes());
            socket.getOutputStream().flush();
            CompletableFuture<Message> futureResponse = new CompletableFuture<>();
            SimpleLogger.debug("tohandle: > " + Message.getCommand(message.getBytes()));
            responseHandlers.put(Message.getCommand(message.getBytes()), futureResponse);
            Message res = futureResponse.get();
            SimpleLogger.debug("Response to the send message: > " + new String(res.getBytes()));
            return res;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    private void handleServerResponse(Message response, String type) {
        CompletableFuture<Message> responseFuture = responseHandlers.remove(type);
        SimpleLogger.debug("Handling server response");
        if (responseFuture != null) {
            responseFuture.complete(response);
        }
    }

    public BufferMap send_interested(String filekey, int numPieces) {
        InterestedMessage interested = new InterestedMessage(filekey);
        Message response = send(interested);
        return InterestedMessage.interestedResponse(response, numPieces);
    }

    public HashMap<Integer, byte[]> send_getpieces(String filekey, BufferMap bm) {
        SimpleLogger.debug("ASKING FOR PIECES");
        ManagedFile file = peer.look_key(filekey);

        // Gérer le cas où on demande plus de piece que ce qu'un message peut contenir
        int nb_asked = bm.getOnes();
        int max_to_ask = (msg_max_size / file.getPieceLength()) - 1; // -1 pour laisser de la place au header et autres

        if (nb_asked > max_to_ask) {
            HashMap<Integer, byte[]> pieces = new HashMap<>();
            SimpleLogger.debug("CUTTING UP");
            ArrayList<BufferMap> sliced_bm = bm.slice(max_to_ask);
            for (BufferMap b : sliced_bm) {
                HashMap<Integer, byte[]> part_pieces = _send_getpieces_(filekey, b, file.getLength(),
                        file.getPieceLength());
                pieces.putAll(part_pieces);
            }
            return pieces;
        }

        return _send_getpieces_(filekey, bm, file.getLength(), file.getPieceLength());

    }

    private HashMap<Integer, byte[]> _send_getpieces_(String filekey, BufferMap bm, int size, int piece_size) {
        GetpiecesMessage getpieces = new GetpiecesMessage(filekey, bm);
        Message response = send(getpieces);
        return GetpiecesMessage.getpiecesResponse(response, bm, Math.min(piece_size, size));
    }

    @Override
    public void run() {
        Thread messageProcessor = new Thread(this::processMessages);
        messageProcessor.start();
        while (true) {
            try {
                byte[] request = new byte[msg_max_size];
                int bytesRead = socket.getInputStream().read(request);

                if (bytesRead != -1) {
                    // Add request to the queue
                    SimpleLogger.debug("Adding msg to queue ");
                    messageQueue.offer(Arrays.copyOf(request, bytesRead));
                }
            } catch (Exception e) {
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
        SimpleLogger.debug("Processing request: " + new String(request));
        try {
            String cmd = Message.getCommand(request);
            switch (cmd) {
                case "have":
                    processHave(request);
                    break;
                case "data":
                    processData(request);
                    break;
                default:
                    SimpleLogger.debug("Unknown command");
                    break;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void processData(byte[] request) {
        handleServerResponse(Message.fromBytes(request), "getpieces");
    }

    private void processHave(byte[] request) throws Exception {
        handleServerResponse(Message.fromBytes(request), "interested");

    }
}
