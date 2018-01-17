// grep the afterCreate and afterUpdate from a single vm_*.log file into a file named "log.txt" and run
// this program to see if there are updates w/o a corresponding create
import java.io.*;
import java.util.*;
public class checkCreates {
  public static void main(String args[]) throws Exception {
    Set updatedKeys = new HashSet();
    Set createdKeys = new HashSet();
    File inputFile = new File("log.txt");
    FileReader fr = new FileReader(inputFile);
    BufferedReader br = new BufferedReader(fr);
    for (String line = br.readLine(); line != null && line.trim().length() > 0; line = br.readLine()) {
      String parts[] = line.split("\\s");
      if (parts.length < 6) {
        continue;
      }
      try {
        int i;
        for (i=0; i<parts.length; i++) {
          if (parts[i].equals("key")) {
            break;
          }
        }
        if (i >= parts.length) {
          continue;
        }
        String key = parts[i+1];
        key = key.substring(0, key.length()-1);
        String operation = parts[i+2];
        if (operation.equals("afterCreate")) {
          createdKeys.add(key);
        }
        else if (operation.equals("afterUpdate")) {
          updatedKeys.add(key);
        }
      }
      catch (ArrayIndexOutOfBoundsException e) {
        e.printStackTrace();
        for (int i=0; i<parts.length; i++) {
          System.out.println("part["+i+"] = '" + parts[i] + "'");
        }
        return;
      }
    }
    System.out.println("found " + createdKeys.size() + " created keys");
    System.out.println("and " + updatedKeys.size() + " updated keys");
    updatedKeys.removeAll(createdKeys);
    for (Iterator it=updatedKeys.iterator(); it.hasNext(); ) {
      System.out.println("Update event but no create event for key " + it.next());
    }
  }
}
