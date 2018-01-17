#!/bin/sh
if [ ! -r merged.txt ]; then
  mergelogs >merged.txt
fi
grep addObjectViaPutAll: merged.txt >log.txt
grep addObject: merged.txt >>log.txt
grep afterCreate merged.txt >>log.txt
java -cp ..:$CLASSPATH checkClientCreates >missing.txt
