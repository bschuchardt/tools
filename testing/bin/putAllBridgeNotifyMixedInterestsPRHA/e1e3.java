import java.io.*;
import java.util.*;
public class e1e3 {
  public static void main(String args[]) throws Exception {
    Map remotes = new HashMap();
    Map origins = new HashMap();
    File inputFile = new File("log.txt");
    FileReader fr = new FileReader(inputFile);
    BufferedReader br = new BufferedReader(fr);
    int e1Count = 0;
    int e3Count = 0;
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
        String operation = parts[i+2];
        if (!operation.equals("afterUpdate")) {
          continue;
        }
        String key = parts[i+1];
        key = key.substring(0, key.length()-1);
        String clientName = parts[i+4];
        if (key.equals("Object_41626")) {
          if (clientName.equals("edge1")) {
            e1Count++;
            System.out.println("["+e1Count+"]  " + line);
          }
          else if (clientName.equals("edge3")) {
            e3Count++;
            System.out.println("<"+e3Count+">  " + line);
          }
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
  }
  
}
