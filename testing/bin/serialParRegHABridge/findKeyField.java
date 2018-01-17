import java.util.*;
import java.io.*;

public class findKeyField {
  public static void main(String args[]) throws Exception {
    BufferedReader reader = 
	    new BufferedReader(new InputStreamReader(System.in));
    for (;;) {
      String line = reader.readLine();
      if (line == null) {
        return;
      }
      String parts[] = line.split("\\s");
      boolean found = false;
      int idx = 0;
      for (int i=0 ; !found && i<parts.length; i++) {
        if (parts[i].equals("key")) {
          found = true;
          idx = i;
          break;
        }
      }
      if (found) {
        String key = parts[idx+1];
        System.out.println(key.substring(0, key.length()-1));
      }
    }
  }
}
