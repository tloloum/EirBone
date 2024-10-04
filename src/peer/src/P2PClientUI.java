import javafx.application.Application;
import javafx.beans.property.SimpleStringProperty;
import javafx.concurrent.Task;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.stage.Stage;
import javafx.util.Callback;
import javafx.scene.layout.*;

public class P2PClientUI extends Application implements UIinterface {
    private static Peer mainPeer;
    private static int mainPeerPort;
    private int seedNb = 0;
    private int leechNb = 0;
    // private String trackerStatus = "NOK";
    private FileLibrary downloadFl;

    public static void setPort(int port) {
        mainPeerPort = port;
    }

    private void initPeer() throws Exception {
        mainPeer = new Peer(mainPeerPort);
        Thread p = new Thread(mainPeer);
        p.start();
        Thread.sleep(1000);
        // mainPeer.pingTracker();
        // mainPeer.announce();
    }

    public void updateStatus() {
        seedNb = mainPeer.getClientCount();
        leechNb = mainPeer.getServerCount();
        // try {
        //     mainPeer.update();
        //     trackerStatus = "OK";
        // } catch (Exception e) {
        //     trackerStatus = "NOK";
        // }
    }

    @SuppressWarnings({ "deprecation", "unchecked" })
    @Override
    public void start(Stage primaryStage) {
        try {
            initPeer();
        } catch (Exception e) {
            e.printStackTrace();
        }
        // Set up the main UI
        BorderPane root = new BorderPane();

        // Downloads Section
        VBox downloadsSection = new VBox(5);
        downloadsSection.setPadding(new Insets(10));

        // // File table
        TableView<String> FileDownloadsTable = new TableView<>();
        TableColumn<String, String> columnFiles = new TableColumn<>("Reseeding Archive");
        columnFiles.setCellValueFactory(data -> new SimpleStringProperty(data.getValue()));

        TableColumn<String, String> columnAction = new TableColumn<>("Action");
        columnAction.setCellValueFactory(data -> new SimpleStringProperty(data.getValue()));

        // Here we set the CellFactory to create a cell which contains a button
        columnAction.setCellFactory(new Callback<TableColumn<String, String>, TableCell<String, String>>() {
            @Override
            public TableCell<String, String> call(TableColumn<String, String> param) {
                return new TableCell<String, String>() {
                    final Button btn = new Button("Télécharger");

                    @Override
                    public void updateItem(String item, boolean empty) {
                        super.updateItem(item, empty);
                        if (empty) {
                            setGraphic(null);
                        } else {
                            btn.setOnAction(event -> {
                                SimpleLogger.debug("Downloading  " + item + " filekey ");
                                // String filekey = mainPeer.sendLook("filename=" + '"'+item+'"').get;
                    
                                startFileDownload(item, downloadsSection);
                            });
                            setGraphic(btn);
                        }
                    }
                };
            }
        });

        FileDownloadsTable.getColumns().addAll(columnFiles, columnAction);
        FileDownloadsTable.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY); // deprecated

        // Place sections in the BorderPane
        root.setCenter(FileDownloadsTable);
        root.setRight(downloadsSection);

        // Status Section
        VBox statusSection = new VBox(5);
        statusSection.setPadding(new Insets(10));
        Label trackerLabel = new Label("Tracker");
        Label seedLabel = new Label("Seed: " + seedNb);
        Label leechLabel = new Label("Leech: " + leechNb);
        Button lookButton = new Button("Look");
        lookButton.setOnAction(event -> {
            try {
                downloadFl = mainPeer.look("*");
                FileDownloadsTable.getItems().addAll(downloadFl.getManagedFilenames());
            } catch (Exception e) {
                e.printStackTrace();
            }
        });
        statusSection.getChildren().addAll(trackerLabel, seedLabel, leechLabel, lookButton);

        root.setLeft(statusSection);

        // Set the scene and stage
        Scene scene = new Scene(root, 800, 600);
        primaryStage.setTitle("Eirbone client");
        primaryStage.setScene(scene);
        primaryStage.show();
    }

    private void startFileDownload(String fileId, VBox downloadsSection) {
        // Check if the file is already being downloaded
        boolean isAlreadyDownloading = downloadsSection.getChildren().stream()
                .anyMatch(node -> node instanceof HBox &&
                        ((Label) ((HBox) node).getChildren().get(0)).getText().equals(fileId));

        if (!isAlreadyDownloading) {
            // Setup the UI elements
            HBox fileProgress = new HBox(5);
            Label fileLabel = new Label(fileId);
            ProgressBar fileProgressBar = new ProgressBar(0);
            Label filePercentLabel = new Label("0%");
            fileProgress.getChildren().addAll(fileLabel, fileProgressBar, filePercentLabel);
            downloadsSection.getChildren().add(fileProgress);

            // Create a task for the download
            Task<Void> downloadTask = new Task<Void>() {
                @Override
                protected Void call() throws Exception {
                    final int totalWork = 100; // This should be the total size of the file
                    int workDone = 0;
                    ManagedFile dlf = downloadFl.getByName(fileId);
                    SimpleLogger.debug("Downloading " + fileId + " " + dlf.getKey() + " " + dlf.getLength() + " " + dlf.getPieceLength());
                    mainPeer.download(dlf);
                    while (workDone < totalWork) {
                        workDone++;
                        updateProgress(workDone, totalWork);
                        updateMessage(String.format("%d%%", (workDone * 100 / totalWork)));

                        // Simulate some delay
                        try {
                            Thread.sleep(100); // Sleep for 100 milliseconds
                        } catch (InterruptedException e) {
                            if (isCancelled()) {
                                updateMessage("Cancelled");
                                break;
                            }
                        }
                    }
                    return null;
                }
            };

            // Bind the progress bar and label to the task's progress and message
            fileProgressBar.progressProperty().bind(downloadTask.progressProperty());
            filePercentLabel.textProperty().bind(downloadTask.messageProperty());
            // Start the task on a new thread
            Thread thread = new Thread(downloadTask);
            thread.setDaemon(true); // Mark the thread as a daemon thread
            thread.start();

        } else {
            SimpleLogger.debug("File " + fileId + " is already downloading.");
        }
    }
}