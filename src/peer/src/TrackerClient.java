import java.net.Socket;

public class TrackerClient implements Client{
    private Socket socket;

    TrackerClient(String ip, int port) {
        try {
            this.socket = new Socket(ip, port);
        } catch (Exception e) {
            e.printStackTrace();
            SimpleLogger.error("TrackerClient: Could not connect to tracker" + ip + ":" + port);
        }
    }

    public Message send(Message message) {
        try {
            socket.getOutputStream().write(message.getBytes());
            SimpleLogger.debug("Tracker: > " + new String(message.getBytes()));
            byte[] response = new byte[1024 * 16];
            socket.getInputStream().read(response);
            SimpleLogger.debug("Tracker: < " + new String(response));
            return Message.fromBytes(response);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}
