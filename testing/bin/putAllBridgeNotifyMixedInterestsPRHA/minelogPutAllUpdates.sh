#!/bin/sh
if [ ! -r merged.txt ]; then
  mergelogs >merged.txt
fi
grep updateObjectViaPutAll: merged.txt >log.txt
grep updateObject: merged.txt >>log.txt
grep afterUpdate merged.txt >>log.txt
java -cp ..:$CLASSPATH checkClientPutAllUpdates >missing.txt
