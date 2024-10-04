import javafx.application.Application;

public class Main {

    public static void main(String[] args) throws Exception {
        int port = NetworkUtils.chooseDefaultPort();
        String currentDir = System.getProperty("user.dir");
        System.out.println("Current working directory: " + currentDir);
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-p") && i + 1 < args.length) {
                try {
                    port = Integer.parseInt(args[i + 1]);
                    i++; // skip next argument as it's the value for port
                } catch (NumberFormatException e) {
                    System.err.println("Invalid port number: " + args[i + 1]);
                    System.exit(1);
                }
            } else {
                System.err.println("Invalid option: " + args[i]);
                System.exit(1);
            }
        }
        P2PClientUI.setPort(port);
        Application.launch(P2PClientUI.class, args);

        System.out.println("Terminate with Ctrl+C");
        System.exit(0);
    }
}
