// grep the afterCreate events from vm*client*.log into a file, then sort on field 15
// before running this program on the result.  Args are filename, expected-creates-per-key
import java.io.*;
import java.util.*;
public class checkCreates {
  public static void main(String args[]) throws Exception {
    int expectedPerKey = Integer.parseInt(args[1]);
    Map<String,Integer> createdKeys = new HashMap();
    File inputFile = new File(args[0]);
    FileReader fr = new FileReader(inputFile);
    BufferedReader br = new BufferedReader(fr);
    for (String line = br.readLine(); line != null && line.trim().length() > 0; line = br.readLine()) {
      String parts[] = line.split("\\s");
      if (parts.length < 6 || !parts[6].equals("Updater")) {
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
          Integer lastCount = createdKeys.get(key);
          Integer nextCount;
          if (lastCount == null) {
            nextCount = Integer.valueOf(1);
          } else {
            nextCount = Integer.valueOf(lastCount.intValue()+1);
          }
          createdKeys.put(key, nextCount);
        }
//        else if (operation.equals("afterUpdate")) {
//          updatedKeys.add(key);
//        }
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
    for (Map.Entry<String,Integer> entry: createdKeys.entrySet()) {
      int count = entry.getValue().intValue();
      if (count != expectedPerKey) {
        System.out.println("Found only " + count + " creates for " + entry.getKey());
      }
    }
  }
}
