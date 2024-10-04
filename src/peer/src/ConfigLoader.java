import java.io.*;
import java.util.Properties;

public class ConfigLoader {
    private static final String CONFIG_FILE = "config/config.ini";
    private static final String TRACKER_IP = "tracker-ip";
    private static final String TRACKER_PORT = "tracker-port";
    private static final String DWNLOAD_DIR = "download-dir";
    private static final String MSG_MAX_SIZE = "msg-max-size";

    private String tracker_ip;
    private int tracker_port;
    private String download_dir;
    private String files_to_share;
    private int msg_max_size;
    private int max_connections_in;
    private int max_connections_out;
    private int piece_size;
    private String logLevel;

    public ConfigLoader() {
        try (InputStream input = new FileInputStream(CONFIG_FILE)) {
            Properties prop = new Properties();
            prop.load(input);

            tracker_ip = prop.getProperty(TRACKER_IP);
            tracker_port = Integer.parseInt(prop.getProperty(TRACKER_PORT));
            download_dir = prop.getProperty(DWNLOAD_DIR);
            files_to_share = prop.getProperty("shared-dir");
            msg_max_size = Integer.parseInt(prop.getProperty(MSG_MAX_SIZE));
            max_connections_in = Integer.parseInt(prop.getProperty("max-connexions-in"));
            max_connections_out = Integer.parseInt(prop.getProperty("max-connexions-out"));
            piece_size = Integer.parseInt(prop.getProperty("piece-size"));
            logLevel = prop.getProperty("log-level");
            SimpleLogger.setLogToConsole(logLevel);

        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    public String getLogLevel() {
        return logLevel;
    }

    public String getTrackerIp() {
        return tracker_ip;
    }

    public int getTrackerPort() {
        return tracker_port;
    }

    public String getDownloadDir() {
        return download_dir;
    }

    public String getFilesFolder() {
        return files_to_share;
    }

    public int getMsgMaxSize() {
        return msg_max_size;
    }

    public int getMaxConnectionsIn() {
        return max_connections_in;
    }

    public int getMaxConnectionsOut() {
        return max_connections_out;
    }

    public int getPieceSize() {
        return piece_size;
    }

}
