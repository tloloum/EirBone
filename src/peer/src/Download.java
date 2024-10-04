import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;

public class Download implements Runnable {
    private ManagedFile file;
    private Peer mainPeer;
    private int completion;
    private int maxCompletion;

    public Download(ManagedFile file, Peer mainPeer) {
        this.file = file;
        this.mainPeer = mainPeer;
        this.maxCompletion = file.getNumPieces();
        this.completion = 0;
    }

    @Override
    public void run() {
        download();
    }

    public int download_state() {
        return completion;
    }

    private void download() {
        String filekey = file.getKey().trim();
        String name = file.getName();
        int length = file.getLength();
        int pieceLength = file.getPieceLength();
        pieceLength = (pieceLength > length) ? length : pieceLength;
        ManagedFile dl = new ManagedFile(name, length, pieceLength, filekey, true);
        HashMap<String, BufferMap> peerBuffermaps = new HashMap<>();
        String[] useful_peers = mainPeer.getfile(filekey);
        for (String peer : useful_peers) {
            // plan downloads
            try {
                String[] ip_port = peer.split(":");
                String portString = ip_port[1].trim();
                int portNumber = Integer.parseInt(portString);
                int id = mainPeer.connect(ip_port[0], portNumber);
                int numPieces = dl.getNumPieces();
                BufferMap bm = mainPeer.interested(filekey, numPieces, id);
                peerBuffermaps.put(peer, bm);
            } catch (Exception e) {
                SimpleLogger.error(name + " download failed");
                e.printStackTrace();
            }
        }
        HashMap<String, BufferMap> downloadPlans = planDownloads(peerBuffermaps, dl.getNumPieces());
        for (String peer : useful_peers) {
            if (dl.getBufferMap().isFull()) {
                SimpleLogger.info(name + " has been downloaded");
                return;
            }
            String[] ip_port = peer.split(":");
            String portString = ip_port[1].trim().replace("\n", "");
            try {
                int portNumber = Integer.parseInt(portString);
                SimpleLogger.debug("Connecting to " + ip_port[0] + " on port " + portNumber);
                int id = mainPeer.connect(ip_port[0], portNumber);
                BufferMap bm = downloadPlans.get(peer);
                HashMap<Integer, byte[]> pieces = mainPeer.getpieces(filekey, bm, id);
                for (int idx : pieces.keySet()) {
                    byte[] piece = pieces.get(idx);
                    dl.set_piece(idx, piece);
                    completion++;
                    SimpleLogger.debug("Downloaded piece " + idx + " " + new String(piece));
                    SimpleLogger.info(completion + "/" + maxCompletion + " pieces downloaded");
                }
            } catch (Exception e) {
                SimpleLogger.error(name + " download failed");
                e.printStackTrace();
            }
        }
    }

    private static HashMap<String, BufferMap> planDownloads(HashMap<String, BufferMap> peerBuffermaps, int numPieces) {
        HashMap<String, BufferMap> downloadPlans = new HashMap<>();
        HashMap<String, Integer> peerPieceCounts = new HashMap<>();

        for (Map.Entry<String, BufferMap> entry : peerBuffermaps.entrySet()) {
            String key = entry.getKey();
            BufferMap bufferMap = new BufferMap(numPieces);
            downloadPlans.put(key, bufferMap);
            peerPieceCounts.put(key, 0);
        }
        for (int i = 0; i < numPieces; i++) {
            String[] peers_that_have_piece = new String[peerBuffermaps.keySet().size()]; // get peers that have piece i
            int count = 0;
            for (Map.Entry<String, BufferMap> entry : peerBuffermaps.entrySet()) {
                String key = entry.getKey();
                BufferMap bufferMap = entry.getValue();
                if (bufferMap.get(i) == 1) {
                    peers_that_have_piece[count] = key;
                    count++;
                }

            }
            SimpleLogger.debug("Piece " + i + " " + Arrays.toString(peers_that_have_piece) + " " + count);
            if (count == 1) {
                downloadPlans.get(peers_that_have_piece[0]).set(i);
                peerPieceCounts.put(peers_that_have_piece[0], peerPieceCounts.get(peers_that_have_piece[0]) + 1);
            } else if (count > 1) {
                Arrays.sort(peers_that_have_piece, 0, count, new Comparator<String>() {
                    @Override
                    public int compare(String o1, String o2) {
                        return peerPieceCounts.get(o1) - peerPieceCounts.get(o2);
                    }
                });
                downloadPlans.get(peers_that_have_piece[0]).set(i);
                peerPieceCounts.put(peers_that_have_piece[0], peerPieceCounts.get(peers_that_have_piece[0]) + 1);
            }
        }
        return downloadPlans;
    }

}
