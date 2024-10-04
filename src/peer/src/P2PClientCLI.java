import java.util.Scanner;

public class P2PClientCLI implements UIinterface {
    private static Peer mainPeer;
    private static int mainPeerPort;
    private int seedNb = 0;
    private int leechNb = 0;
    private FileLibrary downloadFl = new FileLibrary();

    private void initPeer() throws Exception {
        mainPeerPort = NetworkUtils.chooseDefaultPort();
        mainPeer = new Peer(mainPeerPort);
        Thread p = new Thread(mainPeer);
        p.start();
        Thread.sleep(1000);
    }

    public void updateStatus() {
        seedNb = mainPeer.getClientCount();
        leechNb = mainPeer.getServerCount();
    }

    public void start() {
        try {
            initPeer();
        } catch (Exception e) {
            e.printStackTrace();
        }

        Scanner scanner = new Scanner(System.in);
        String command;

        System.out.println("Eirbone client CLI");
        System.out.println("Type 'help' for a list of commands.");

        while (true) {
            System.out.print("> ");
            command = scanner.nextLine();

            if (command.equalsIgnoreCase("exit")) {
                System.out.println("Exiting...");
                break;
            } else if (command.equalsIgnoreCase("status")) {
                updateStatus();
                System.out.println("Seed: " + seedNb);
                System.out.println("Leech: " + leechNb);
            } else if (command.equalsIgnoreCase("look")) {
                try {
                    downloadFl = mainPeer.look("*");
                    String[] filenames = downloadFl.getManagedFilenames();
                    System.out.println("Available files:");
                    for (String filename : filenames) {
                        System.out.println(" - " + filename);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else if (command.startsWith("download ")) {
                String fileId = command.substring(9).trim();
                startFileDownload(fileId);
            } else if (command.equalsIgnoreCase("help")) {
                printHelp();
            } else {
                System.out.println("Unknown command. Type 'help' for a list of commands.");
            }
        }

        scanner.close();
    }

    private void startFileDownload(String fileId) {
        // Check if the file is already being downloaded
        // Here, we will assume a simple flag for demo purposes. In a real scenario,
        // we should keep track of the downloading files.
        boolean isAlreadyDownloading = false; // Update this logic as necessary

        if (!isAlreadyDownloading) {
            System.out.println("Starting download for " + fileId);

            // Create a new thread for the download
            Thread downloadThread = new Thread(() -> {
                final int totalWork = 10; // This should be the total size of the file
                int workDone = 0;
                try {
                    ManagedFile dlf = downloadFl.getByName(fileId);
                    System.out.println("-> " + fileId + " " + dlf.getKey() + " " + dlf.getLength() + " " + dlf.getPieceLength());
                    mainPeer.download(dlf);
                    while (workDone < totalWork) {
                        workDone++;
                        //System.out.println(String.format("Downloading %s: %d%%", fileId, (workDone * 100 / totalWork)));

                        // Simulate some delay
                        try {
                            Thread.sleep(100); // Sleep for 100 milliseconds
                        } catch (InterruptedException e) {
                            System.out.println("Download interrupted");
                            break;
                        }
                    }
                    System.out.println("Download complete for " + fileId);
                } catch (Exception e) {
                    System.out.println("Error during download: " + e.getMessage());
                    e.printStackTrace();
                }
            });

            // Start the download thread
            downloadThread.setDaemon(true);
            downloadThread.start();

        } else {
            // File is already downloading, handle accordingly (e.g., show an error message)
            System.out.println("File " + fileId + " is already downloading.");
        }
    }

    private void printHelp() {
        System.out.println("Available commands:");
        System.out.println(" - status: Show the number of seeders and leechers");
        System.out.println(" - look: List available files for download");
        System.out.println(" - download <fileId>: Download the specified file");
        System.out.println(" - exit: Exit the application");
    }

    public static void main(String[] args) {
        P2PClientCLI client = new P2PClientCLI();
        client.start();
    }
}
