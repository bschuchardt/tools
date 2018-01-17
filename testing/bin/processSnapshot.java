import java.util.*;
import java.io.*;

public class processSnapshot {
  public static void main(String args[]) throws Exception {
    BufferedReader reader = 
	    new BufferedReader(new InputStreamReader(System.in));
            
    boolean found = false;
    int idx = 0;
    String parts[] = null;
    while (!found) {
      String line = reader.readLine();
      if (line == null) {
        return;
      }
      parts = line.split("\\{");
      System.err.println("parsed first line into " + parts.length + " parts");
      for (int i=0 ; !found && i<parts.length; i++) {
        System.err.println("parts["+i+"]="+parts[i]);
        if (parts[i].startsWith("Object")) {
          found = true;
          idx = i;
          break;
        }
      }
    }
    if (found) {
      processLine(parts[idx]);
      for (;;) {
        String line = reader.readLine();
        if (line == null) {
          return;
        }
        processLine(line);
      }
    }
  }
  public static void processLine(String line) {
    System.err.println("processLine() " + line);
    String parts[] = line.split("\\s");
    for (int i=0; i<parts.length; i++) {
      System.err.println("parts["+i+"]="+parts[i]);
      processKey(parts[i]);
    }
  }
  public static void processKey(String key) {
    if (key.startsWith("Object")) {
      String ps[] = key.split("=");
      System.out.println(ps[0]);
    }
  }
}
