#!/bin/sh
set -v
set -x
grep afterCreate *edge1*.log | java -cp ..:. findKeyField | sort >edge1c.txt
grep afterCreate *edge2*.log | java -cp ..:. findKeyField | sort >edge2c.txt
cat snapshot.txt | java -cp ..:. processSnapshot | sort >snapshot.srt
diff snapshot.srt edge1c.txt
diff snapshot.srt edge2c.txt

