import java.util.*;
import java.io.*;

/** find a missing key in giiWhileMultiplePublishing */

public class findMissingKey {

  public static void main(String args[]) throws Exception {
    File systemFile = new File(args[0]);
    FileReader fr = new FileReader(systemFile);
    BufferedReader input = new BufferedReader(fr);
    
    int currentKey = 0;
    Set allKeys = new HashSet();
    Set allSeqnos = new HashSet();
    System.out.println("Finding log entries for CREATE message processing");
    for (;;) {
      String line = input.readLine();
      if (line == null) break;
      int i = line.indexOf("CREATE");
      if (i < 0) continue;
      i = line.indexOf("Processing");
      if (i < 0) continue;
      i = line.indexOf("UpdateOperation");
      if (i < 0) continue;
      // we have a Processing...CREATE log entry - get the key
      int ki = line.indexOf("key=");
      if (ki < 0) continue;
      int si = line.indexOf(";", ki+4);
      if (si < 0) continue;
      String keyString = line.substring(ki+4, si);
      /*
      int key = Integer.parseInt(keyString);
      System.out.println(keyString);
      if (key > currentKey+1) {
        System.out.println("currentKey="+currentKey
         + " but next key is " + key
         + ".  Missing keys between these two");
        break;
      }
      currentKey = key;
      */
      allKeys.add(keyString);
      int ei = line.indexOf("EventId[");
      si = line.indexOf("];", ei);
      String eventID = line.substring(ei+7, si+1);
      if (allSeqnos.contains(eventID)) {
        System.out.println("Found duplicate eventID: " + eventID);
      }
      allSeqnos.add(eventID);
    }
    System.out.println("Found " + allKeys.size() + " keys");
    if (allKeys.size() != 250000) {
      System.out.println("Looking for missing keys");
      for (int i=0; i<250000; i++) {
        if (!allKeys.contains(String.valueOf(i))) {
          System.out.println("Missing key " + i);
        }
      }
    }
  }

}
