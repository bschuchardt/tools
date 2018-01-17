import java.util.*;
import java.io.*;

public class scan {

  public static void main(String args[]) throws Exception {
    File systemFile = new File(args[0]);
    FileReader fr = new FileReader(systemFile);
    BufferedReader input = new BufferedReader(fr);
    Integer ONE = new Integer(1);
    int lineNumber = 0;
    
//    Map<String, Long> threadPutAllCounts = new HashMap();
    int expected = 0;
    int notifications = 0;
    String notificationMember = null;
    for (;;) {
      String line = input.readLine();
      if (line == null) break;
      lineNumber++;
      int i = 0;
      i = line.indexOf(".log:");
      if (i > 0) {
        notificationMember = line.substring(0, i);
      }
      i = line.indexOf("isPutAll(): true");
      if (i > 0) {
        if (notificationMember.indexOf("edge") < 0) {
          notifications++;
        }
        continue;
      }
      i = line.indexOf("NUM_PUTALL_CREATE is");
      if (i < 0) continue;
      expected = Integer.parseInt(line.substring(i+21, line.length()));
      if (notifications < (expected * 2)) {
        System.out.println("missing notifications at line " + lineNumber
          + ".  Found " + notifications + " but expected " + (expected*2));
//        return;
      }
    }
    System.out.println("expected " + (expected*2) + " putAll notifications and "
      + "received " + notifications);
  }

}
