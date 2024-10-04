import java.io.File;
import java.util.HashMap;

public class FileLibrary {
    private HashMap<String, ManagedFile> files;

    public FileLibrary() {
        this.files = new HashMap<String, ManagedFile>();
    }

    public FileLibrary(String folder) {
        this();
        File[] files = new File(folder).listFiles();
        for (File file : files) {
            ManagedFile mf = new ManagedFile(file.getAbsolutePath());
            this.files.put(mf.getKey(), mf);
        }
    }

    public void addFile(ManagedFile file) {
        files.put(file.getKey(), file);
    }

    public String announce() {
        String a = "[";
        for (String key : files.keySet()) {
            if (!files.get(key).getName().equals(".gitkeep")) {
                a += files.get(key).announce() + " ";
            }
        }
        a += "]";
        return a;
    }



    public ManagedFile get(String key) {
        for (String k : files.keySet()) {
            if (k.equals(key.trim())) {
                return files.get(k);
            }
        }
        return null;
    }

    //Should not be used, can be multiple files with the same name
    public ManagedFile getByName(String name) {
        for (String k : files.keySet()) {
            if (files.get(k).getName().equals(name.trim())) {
                return files.get(k);
            }
        }
        return null;
    }

    public String[] getKeys() {
        return files.keySet().toArray(new String[0]);
    }
    
    public String[] getManagedFilenames() {
        String[] keys = files.keySet().toArray(new String[0]);
        String[] filenames = new String[keys.length];
        for (int i = 0; i < keys.length; i++) {
            filenames[i] = files.get(keys[i]).getName();
        }
        return filenames;
    }

    public ManagedFile[] getAll() {
        return files.values().toArray(new ManagedFile[0]);
    }

    @Override
    public String toString() {
        if (isEmpty()) {
            return "\033[1;31mThe file library is currently empty.\033[0m";
        }

        // Header with ASCII Art
        String header = "\n   ______________ \n < FILE LIBRARY >\n   -------------- \n";
        String underline = "-----------------------------------------------------\n";
        StringBuilder sb = new StringBuilder();
        sb.append(header);

        // Display summary of total files in the library
        sb.append(String.format(" \033[1;36mTotal ManagedFiles\033[0m: \033[1;33m%d\033[0m\n", files.size()));
        sb.append(underline);

        // Use the ManagedFile class's toString method to append each file's detailed
        // description
        for (ManagedFile file : files.values()) {
            sb.append(file.toString()).append("\n");
            sb.append(underline); // Optional: add underline after each file for better separation
        }

        return sb.toString();
    }

    public boolean isEmpty() {
        return files.isEmpty();
    }
}
