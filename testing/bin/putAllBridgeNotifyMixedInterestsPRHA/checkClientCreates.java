import java.io.*;
import java.util.*;
public class checkClientCreates {
  public static void main(String args[]) throws Exception {
    Map remotes = new HashMap();
    Set origins = new HashSet();
    File inputFile = new File("log.txt");
    FileReader fr = new FileReader(inputFile);
    BufferedReader br = new BufferedReader(fr);
    for (String line = br.readLine(); line != null && line.trim().length() > 0; line = br.readLine()) {
      String parts[] = line.split("\\s");
      if (parts.length < 6) {
        continue;
      }
      try {
        if (parts[7].equals("addObjectViaPutAll:")) {
          String key = parts[14];
          key = key.substring(0, key.length()-1);
          origins.add(key);
          continue;
        }
        if (parts[7].equals("addObject:")) {
          String key = parts[12];
          key = key.substring(0, key.length()-1);
          origins.add(key);
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
        if (!operation.equals("afterCreate")) {
          continue;
        }
        String key = parts[i+1];
        key = key.substring(0, key.length()-1);
        String clientName = parts[i+4];
        //if (clientName.equals("edge1")) {
          Set clientKeys = (Set)remotes.get(clientName);
          if (clientKeys == null) {
            clientKeys = new HashSet();
            remotes.put(clientName, clientKeys);
          }
          clientKeys.add(key);
        //}
      }
      catch (ArrayIndexOutOfBoundsException e) {
        e.printStackTrace();
        for (int i=0; i<parts.length; i++) {
          System.out.println("part["+i+"] = '" + parts[i] + "'");
        }
        return;
      }
    }
    //LinkedList l = new LinkedList(origins);
    //Collections.sort(l);
    System.out.println(" found " + origins.size() + " put/putAll keys");
    for (Iterator it=remotes.entrySet().iterator(); it.hasNext();) {
      Map.Entry entry = (Map.Entry)it.next();
      String clientName = (String)entry.getKey();
      Set clientKeys = (Set)entry.getValue();
      System.out.println(" "+clientName+" has "+clientKeys.size()+" creates");
    }
    for (Iterator it=origins.iterator(); it.hasNext(); ) {
      String key = (String)it.next();
      int keyDigit = key.charAt(key.length()-1) - '0';
      for (Iterator rit = remotes.entrySet().iterator(); rit.hasNext(); ) {
        Map.Entry entry = (Map.Entry)rit.next();
        String clientName = (String)entry.getKey();
        if (clientName.equals("edge1") ||
          (clientName.equals("edge2") && (keyDigit%2 == 1)) ||
          (clientName.equals("edge3") && (keyDigit%2 == 0)) ) {
          Set clientKeys = (Set)entry.getValue();
          if (!clientKeys.contains(key)) {
            System.out.println(clientName + " missing key " + key);
          }
        }
      }
    }
    /*
    for (Iterator rit = remotes.entrySet().iterator(); rit.hasNext(); ) {
      Map.Entry entry = (Map.Entry)rit.next();
      Set clientKeys = (Set)entry.getValue();
      String clientName = (String)entry.getKey();
      for (Iterator it = clientKeys.iterator(); it.hasNext(); ) {
        String key = (String)it.next();
        if (!origins.contains(key)) {
          System.out.println(clientName + " has extra key " + key);
        }
      }
    }
    */
  }
}
