#!/bin/sh
mergelogs >merged.txt
grep updateObjectViaPutAll: merged.txt >log.txt
grep updateObject: merged.txt >>log.txt
grep afterUpdate merged.txt >>log.txt
java -cp ..:$CLASSPATH checkClientUpdates >missing.txt
