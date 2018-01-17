import java.io.*;
import java.util.*;
public class checkClientPutAllUpdates {
  public static void main(String args[]) throws Exception {
    Map remotes = new HashMap();
    Map origins = new HashMap();
    File inputFile = new File(args.length>0? args[0] : "log.txt");
    FileReader fr = new FileReader(inputFile);
    BufferedReader br = new BufferedReader(fr);
    for (String line = br.readLine(); line != null && line.trim().length() > 0; line = br.readLine()) {
      String parts[] = line.split("\\s");
      if (parts.length < 6) {
        continue;
      }
      try {
        if (parts[7].equals("updateObjectViaPutAll:")) {
          String key = parts[14];
          key = key.substring(0, key.length()-1);
          updateCount(origins, key);
          continue;
        }
        if (parts[7].equals("updateObject:")) {
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
        String event = parts[i+5];
        if (event.indexOf("PUTALL") < 0) {
          continue;
        }
        String key = parts[i+1];
        key = key.substring(0, key.length()-1);
        String clientName = parts[i+4];
//        if (clientName.startsWith("edge")) {
          Map clientKeys = (Map)remotes.get(clientName);
          if (clientKeys == null) {
            clientKeys = new HashMap();
            remotes.put(clientName, clientKeys);
          }
          updateCount(clientKeys, key);
//        }
      }
      catch (ArrayIndexOutOfBoundsException e) {
        e.printStackTrace();
        if (parts.length == 0) {
          System.out.println("parts length is zero");
        }
        for (int i=0; i<parts.length; i++) {
          System.out.println("part["+i+"] = '" + parts[i] + "'");
        }
        return;
      }
    }
    System.out.println("found " + origins.size() + " keys updated via putAll");
    int totalUpdates = 0;
    for (Iterator it=origins.values().iterator(); it.hasNext(); ) {
      Integer updates = (Integer)it.next();
      totalUpdates += updates.intValue();
    }
    System.out.println("Total updates performed was " + totalUpdates);
    
    int combinedEdges = 0;
    int combinedBridges = 0;
    for (Iterator it=remotes.entrySet().iterator(); it.hasNext(); ) {
      Map.Entry remoteEntry = (Map.Entry)it.next();
      int total = 0;
      Map clientUpdates = (Map)remoteEntry.getValue();
      String clientName = (String)remoteEntry.getKey();
      for (Iterator ui = clientUpdates.values().iterator(); ui.hasNext(); ) {
        Integer updates = (Integer)ui.next();
        total += updates.intValue();
      }
      System.out.println(""+remoteEntry.getKey()+": " + total + " updates");
      if (clientName.startsWith("edge")) {
        combinedEdges += total;
      }
      else {
        combinedBridges += total;
      }
    }
    
    System.out.println("Total update events recorded by edges: " + combinedEdges);
    System.out.println("Total update events recorded by bridges: " + combinedBridges);
    
    boolean debug=false;
    Map allBridgeCounts = new HashMap();
    for (Iterator it=origins.entrySet().iterator(); it.hasNext(); ) {
      Map.Entry oe = (Map.Entry)it.next();
      String key = (String)oe.getKey();
      int keyDigit = key.charAt(key.length()-1) - '0';
      
      Integer updateCount = (Integer)oe.getValue();

      if (debug) System.out.println("Checking " + key + " expecting " + updateCount);
      
      int bridgeTotalForKey = 0;

      for (Iterator rit = remotes.entrySet().iterator(); rit.hasNext(); ) {
        Map.Entry entry = (Map.Entry)rit.next();
        String clientName = (String)entry.getKey();
        Map clientUpdates = (Map)entry.getValue();
        if (clientName.equals("edge1") ||
           (clientName.equals("edge2") && (keyDigit%2 == 1)) ||
           (clientName.equals("edge3") && (keyDigit%2 == 0)) ) {
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
        else if (clientName.startsWith("bridge")) {
          Integer bridgeCount = (Integer)clientUpdates.get(key);
          if (bridgeCount != null) {
            clientUpdates.remove(key); // REMOVE SO WE CAN SEE WHAT'S LEFT OVER
            bridgeTotalForKey += bridgeCount.intValue();
            //if (key.equals("Object_16252")) {
            //  System.out.println(clientName + " has " + bridgeCount + " updates for " + key);
            //}
          }
        }
      }
      allBridgeCounts.put(key, bridgeTotalForKey);
    }
    
    if (totalUpdates != combinedBridges) {
      int newTotalUpdates = 0;
      int newCombinedBridges = 0;
      for (Iterator it=origins.entrySet().iterator(); it.hasNext(); ) {
        Map.Entry oe = (Map.Entry)it.next();
        String key = (String)oe.getKey();
        Integer updateCount = (Integer)oe.getValue();
        Integer bridgeCount = (Integer)allBridgeCounts.get(key);
        newTotalUpdates += updateCount;
        newCombinedBridges += bridgeCount;
        if (!updateCount.equals(bridgeCount)) {
          System.out.println("Bridges have " + bridgeCount + " updates for " + key + " but was expecting " + updateCount);
        }
        allBridgeCounts.remove(key);
      }
      if (allBridgeCounts.size() > 0) {
        System.out.println("Update notification without corresponding putAll put for these keys:");
        for (Iterator it=allBridgeCounts.entrySet().iterator(); it.hasNext(); ) {
          Map.Entry e = (Map.Entry)it.next();
          String key = (String)e.getKey();
          Integer count = (Integer)e.getValue();
          System.out.println("  "+key+"  update count="+count);
        }
      }
      if (newCombinedBridges != combinedBridges) {
        System.out.println("looking for extra events in bridges");
        for (Iterator it=remotes.entrySet().iterator(); it.hasNext(); ) {
          Map.Entry entry = (Map.Entry)it.next();
          String name = (String)entry.getKey();
          if (name.startsWith("bridge")) {
            Map updates = (Map)entry.getValue();
            for (Iterator ui=updates.entrySet().iterator(); ui.hasNext(); ) {
              Map.Entry ue = (Map.Entry)ui.next();
              String key = (String)ue.getKey();
              Integer count = (Integer)ue.getValue();
              System.out.println(name + " has update instead of create for key '" + key + "' of count " + count);
            }
          }
        }
      }
      //System.out.println("Total updates performed was " + newTotalUpdates);
      //System.out.println("Total update events recorded by bridges: " + newCombinedBridges);
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
