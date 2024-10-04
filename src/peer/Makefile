# Variables
JAVAC = javac
JAVA_SRC = $(wildcard ./src/*.java)
JAVA_SRC_NOGUI = $(filter-out ./src/P2PClientUI.java ./src/Main.java, $(JAVA_SRC))
JAVA_MODULES = javafx.controls,javafx.fxml
BUILD_DIR = ./build
PATH_TO_FX = ./lib

# Default target
all: peer start

# Java compilation
peer:
	$(JAVAC) -Xlint:deprecation --module-path $(PATH_TO_FX) --add-modules $(JAVA_MODULES) -d $(BUILD_DIR) $(JAVA_SRC)

start:
	java --module-path $(PATH_TO_FX) --add-modules $(JAVA_MODULES) -cp $(BUILD_DIR) Main
test: peer
	java  -cp $(BUILD_DIR) Test
# Clean up
term: peernogui
	java -cp $(BUILD_DIR) P2PClientCLI 
peernogui:
	$(JAVAC) -d $(BUILD_DIR) $(JAVA_SRC_NOGUI)
test-fl : peer
	java -cp $(BUILD_DIR) FileLibraryTst

clean:
	rm -f build/*
	rm -rf downloads/*
