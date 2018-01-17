import java.io.*;
import java.util.*;
public class checkClientUpdates {
  public static void main(String args[]) throws Exception {
    Map remotes = new HashMap();
    Map origins = new HashMap();
    File inputFile = new File("log.txt");
    FileReader fr = new FileReader(inputFile);
    BufferedReader br = new BufferedReader(fr);
    for (String line = br.readLine(); line != null && line.trim().length() > 0; line = br.readLine()) {
      String parts[] = line.split("\\s");
      if (parts.length < 6) {
        continue;
      }
      try {
        if (parts[5].equals("updateObjectViaPutAll:")) {
          String key = parts[12];
          key = key.substring(0, key.length()-1);
          updateCount(origins, key);
          continue;
        }
        if (parts[5].equals("updateObject:")) {
          if (parts.length < 11) {
            continue; // "No names in region"
          }
          String key = parts[8];
          //key = key.substring(0, key.length()-1);
          if (key.equals("with")) {
            continue;
          }
          updateCount(origins, key);
          continue;
        }
        int i;
        for (i=0; i<parts.length; i++) {
          if (parts[i].equals("key")) {
            break;
          }
        }
        if (i >= parts.length) {
          continue;
        }
        String operation = parts[i+2];
        if (!operation.equals("afterUpdate")) {
          continue;
        }
        String key = parts[i+1];
        key = key.substring(0, key.length()-1);
        String clientName = parts[i+4];
        if (clientName.startsWith("edge")) {
          Map clientKeys = (Map)remotes.get(clientName);
          if (clientKeys == null) {
            clientKeys = new HashMap();
            remotes.put(clientName, clientKeys);
          }
          updateCount(clientKeys, key);
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
    System.out.println(" found " + origins.size() + " updated keys");
    int total = 0;
    for (Iterator it=origins.values().iterator(); it.hasNext(); ) {
      Integer updates = (Integer)it.next();
      total += updates.intValue();
    }
    System.out.println("Total updates performed was " + total);
    
    int combinedClients = 0;
    for (Iterator it=remotes.entrySet().iterator(); it.hasNext(); ) {
      Map.Entry remoteEntry = (Map.Entry)it.next();
      total = 0;
      Map clientUpdates = (Map)remoteEntry.getValue();
      for (Iterator ui = clientUpdates.values().iterator(); ui.hasNext(); ) {
        Integer updates = (Integer)ui.next();
        total += updates.intValue();
      }
      System.out.println(""+remoteEntry.getKey()+": " + total + " updates");
      combinedClients += total;
    }
    
    System.out.println("Total update events recorded by clients: " + combinedClients);
    
    boolean debug=false;
    for (Iterator it=origins.entrySet().iterator(); it.hasNext(); ) {
      Map.Entry oe = (Map.Entry)it.next();
      String key = (String)oe.getKey();
      int keyDigit = key.charAt(key.length()-1) - '0';
      Integer updateCount = (Integer)oe.getValue();
      if (debug) System.out.println("Checking " + key + " expecting " + updateCount);
      for (Iterator rit = remotes.entrySet().iterator(); rit.hasNext(); ) {
        Map.Entry entry = (Map.Entry)rit.next();
        String clientName = (String)entry.getKey();
        Map clientUpdates = (Map)entry.getValue();
        if (clientName.equals("edge1") ||
           (key.compareTo("Object_50001") < 0 &&
             ((clientName.equals("edge2") && (keyDigit%2 == 1)) ||
              (clientName.equals("edge3") && (keyDigit%2 == 0)))) ) {
          Integer clientCount = (Integer)clientUpdates.get(key);
          if (debug) System.out.println("  " + clientName + " has " + clientCount);
          if (clientCount == null) {
            System.out.println(clientName + " is missing updates for " + key);
          }
          else {
            if (!updateCount.equals(clientCount)) {
              System.out.println(clientName + " has " + clientCount + " updates for " + key + " but was expecting " + updateCount);
            }
          }
        }
      }
    }
  }
  
  static Integer zero = new Integer(0);
  static void updateCount(Map map, String key) {
    Integer oldValue = zero;
    if (map.containsKey(key)) {
      oldValue = (Integer)map.get(key);
    }
    map.put(key, new Integer(oldValue.intValue()+1));
  }
}
