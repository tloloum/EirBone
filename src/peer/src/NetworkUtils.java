import java.io.IOException;
import java.net.ServerSocket;

public class NetworkUtils {
    public static int chooseDefaultPort() throws IOException {
        try (ServerSocket socket = new ServerSocket(0)) {
            SimpleLogger.info("Chose port " + socket.getLocalPort() + " for listening");
            return socket.getLocalPort();
        }
    }

    public static boolean isValidIPv4(String ip) {
        String[] parts = ip.split("\\.");
        if (parts.length != 4) {
            return false;
        }
        for (String part : parts) {
            try {
                int num = Integer.parseInt(part);
                if (num < 0 || num > 255) {
                    return false;
                }
            } catch (NumberFormatException e) {
                return false;
            }
        }
        return true;
    }
}
