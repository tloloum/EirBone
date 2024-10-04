import java.util.ArrayList;
import java.util.HashMap;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class Peer implements Runnable {
    private int port;
    public TrackerClient tracker_client;
    private ArrayList<PeerClient> peer_clients;
    private ArrayList<PeerServer> peer_servers;
    private FileLibrary fileLibrary;
    private int msg_max_size;
    private int max_connections_in;
    private int max_connections_out;

    public Peer(int port) {
        this.port = port;
        ConfigLoader config = new ConfigLoader();
        SimpleLogger.info("Tracker IP: " + config.getTrackerIp());
        SimpleLogger.info("Tracker Port: " + config.getTrackerPort());
        this.tracker_client = new TrackerClient(config.getTrackerIp(), config.getTrackerPort());
        this.peer_clients = new ArrayList<PeerClient>();
        this.peer_servers = new ArrayList<PeerServer>();
        this.fileLibrary = new FileLibrary();
        this.msg_max_size = config.getMsgMaxSize();
        this.max_connections_in = config.getMaxConnectionsIn();
        this.max_connections_out = config.getMaxConnectionsOut();
    }

    public Peer(int port, String tracker_ip, int tracker_port, String filesFolder) {
        this(port);
        this.fileLibrary = new FileLibrary(filesFolder);
    }

    private void initFileLibrary(String folder) {
        this.fileLibrary = new FileLibrary(folder);
    }

    public void addPeerClient(PeerClient pc) {
        if (peer_clients.size() >= max_connections_in) {
            SimpleLogger.debug("Max connections reached");
            return;
        }
        SimpleLogger.debug(this + " Adding peer client");
        peer_clients.add(pc);
    }

    public void removePeerClient(PeerClient pc) {
        SimpleLogger.debug(this + " Removing peer client");
        peer_clients.remove(pc);
    }

    public void addPeerServer(PeerServer ps) {
        if (peer_servers.size() >= max_connections_out) {
            SimpleLogger.debug("Max connections reached");
            return;
        }
        SimpleLogger.debug(this + " Adding peer server");
        peer_servers.add(ps);
    }

    public void removePeerServer(PeerServer ps) {
        SimpleLogger.debug(this + " Removing peer server");
        peer_servers.remove(ps);
    }

    public int getClientCount() {
        return peer_clients.size();
    }

    public int getServerCount() {
        return peer_servers.size();
    }

    public int connect(String ip, int port) throws Exception {

        for (PeerClient pc : peer_clients) {
            if (pc.getIp().equals(ip) && pc.getPort() == port) {
                SimpleLogger.debug("Already connected to " + ip + " on port " + port);
                return pc.getId();
            }
        }
        PeerClient client = new PeerClient(ip, port, this, msg_max_size);
        Thread t = new Thread(client);
        t.start();
        return client.getId();
    }

    private void shutdown_clients() {
        for (PeerClient pc : peer_clients) {
            pc.shutdown();
        }
    }

    private void shutdown_servers() {
        for (PeerServer ps : peer_servers) {
            ps.shutdown();
        }
    }

    public Message announce() {
        AnnounceMessage announce = new AnnounceMessage(port, fileLibrary);

        Message response = tracker_client.send(announce);
        if (response == null) {
            SimpleLogger.error("Failed to announce to tracker");
            return null;
        }

        return response;
    }

    public Message update() {
        UpdateMessage update = new UpdateMessage(port, fileLibrary);

        Message response = tracker_client.send(update);
        if (response == null) {
            SimpleLogger.error("Failed to update to tracker");
            return null;
        }

        return response;
    }

    public FileLibrary look(String filename) {
        LookMessage look;
        if (filename == "*") {
            look = new LookMessage("*");
        } else {
            look = new LookMessage("filename=" + '"' + filename + '"');
        }
        return LookMessage.lookResponse(tracker_client.send(look));
    }

    public ManagedFile look_first(String filename) {
        FileLibrary fl = look(filename);
        if (fl.isEmpty()) {
            return null;
        }
        return fl.getAll()[0];
    }

    public ManagedFile look_key(String key) {
        LookMessage lookkey = new LookMessage("key=" + '"' + key + '"');
        return LookMessage.lookResponse(tracker_client.send(lookkey)).get(key);
    }

    public String[] getfile(String filekey) {
        GetfileMessage getfile = new GetfileMessage(filekey);
        return GetfileMessage.getfileResponse(tracker_client.send(getfile));
    }

    public byte[] respond_to_interested(String filekey) {
        // return fileLibrary.get(filekey).getBufferMap();
        BufferMap bm = fileLibrary.get(filekey).getBufferMap();
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        try {
            outputStream.write("have ".getBytes(StandardCharsets.UTF_8)); // Commence par "data "
            outputStream.write(filekey.trim().getBytes(StandardCharsets.UTF_8)); // Ajouter la clé
            outputStream.write(" ".getBytes(StandardCharsets.UTF_8)); // Ajouter un espace
            outputStream.write(bm.toByteArray()); // Ajouter la buffermap
        } catch (IOException e) {
            e.printStackTrace();
        }
        byte[] tosend = outputStream.toByteArray();
        return tosend;
    }

    public BufferMap interested(String filekey, int numPieces, int client_id) {
        SimpleLogger.debug("Sending interested to client " + client_id + " for file " + filekey);
        PeerClient client = peer_clients.get(client_id);
        BufferMap bm = client.send_interested(filekey, numPieces);
        return bm;
    }

    public byte[] respond_to_getpieces(String filekey, int[] id_pieces) {
        ManagedFile file = fileLibrary.get(filekey);

        // Utiliser ByteArrayOutputStream pour gérer l'écriture des données et des
        // textes
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

        try {
            outputStream.write("data ".getBytes(StandardCharsets.UTF_8)); // Commence par "data "
            outputStream.write(filekey.getBytes(StandardCharsets.UTF_8)); // Ajouter la clé
            outputStream.write(" [".getBytes(StandardCharsets.UTF_8)); // Commence par un crochet ouvrant

            for (int j = 0; j < id_pieces.length; j++) {
                byte[] piece = file.get_piece(id_pieces[j]);

                if (j > 0) {
                    outputStream.write(' '); // Ajouter un espace entre les pièces
                }

                // Ajouter le numéro de la pièce et deux points
                String header = id_pieces[j] + ":";
                outputStream.write(header.getBytes(StandardCharsets.UTF_8));

                // Ajouter la pièce de fichier
                outputStream.write(piece);
            }

            outputStream.write(']'); // Terminer par un crochet fermant

        } catch (IOException e) {
            e.printStackTrace();
        }

        byte[] data = outputStream.toByteArray();

        return data;
    }

    public HashMap<Integer, byte[]> getpieces(String filekey, BufferMap id_pieces, int client_id) {
        return peer_clients.get(client_id).send_getpieces(filekey, id_pieces);
    }

    public void download(ManagedFile file) {
        Download dl = new Download(file, this);
        Thread t = new Thread(dl);
        t.start();
    }

    @Override
    public void run() {
        Runtime.getRuntime().addShutdownHook(new Thread() {
            public void run() {
                // Code to be executed when the program is shutting down
                System.out.println("Shutting down...");
                shutdown_clients();
                shutdown_servers();
                // Close connections, release resources, etc.
            }
        });
        // Configure file library
        ConfigLoader cfg = new ConfigLoader();
        initFileLibrary(cfg.getFilesFolder());
        // Start ConnectionsHandler server
        ConnectionsHandler server = new ConnectionsHandler(port, this, msg_max_size);
        Thread t = new Thread(server);
        t.start();

        // Tracker announce
        announce();

    }
}
