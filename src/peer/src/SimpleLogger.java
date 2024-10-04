import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Date;

public class SimpleLogger {
    private static final String LOG_FILE = "./config/peer.log";
    private static final SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    private static LogLevel logToConsole = LogLevel.ERROR; // Flag to control console output

    public enum LogLevel {
        INFO,
        DEBUG,
        ERROR
    }

    public static void setLogToConsole(String lvl) {
        switch (lvl) {
            case "INFO":
                logToConsole = LogLevel.INFO;
                break;
            case "DEBUG":
                logToConsole = LogLevel.DEBUG;
                break;
            case "ERROR":
                logToConsole = LogLevel.ERROR;
                break;
            default:
                logToConsole = LogLevel.ERROR;
        }
    }

    private static void log(LogLevel level, String message) {
        String timestamp = dateFormat.format(new Date());
        String logMessage = String.format("%s %-5s %s", timestamp, level, message);
        
        if (logToConsole.compareTo(level) <= 0) {
            System.out.println(logMessage);
        }

        try (FileWriter fw = new FileWriter(LOG_FILE, true);
             PrintWriter pw = new PrintWriter(fw)) {
            pw.println(logMessage);
        } catch (IOException e) {
            System.err.println("Failed to write to log file: " + e.getMessage());
        }
    }

    public static void info(String message) {
        log(LogLevel.INFO, message);
    }

    public static void debug(String message) {
        log(LogLevel.DEBUG, message);
    }

    public static void error(String message) {
        log(LogLevel.ERROR, message);
    }
}
