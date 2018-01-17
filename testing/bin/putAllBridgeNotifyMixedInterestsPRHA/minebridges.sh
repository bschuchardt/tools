#!/bin/sh
#grep updateObjectViaPutAll: merged.txt | grep _16252 >bridge.txt
#grep afterUpdate merged.txt  | grep bridge | grep _16252 >>bridge.txt
grep updateObjectViaPutAll: merged.txt >bridges.txt
grep afterUpdate merged.txt  | grep bridge >>bridges.txt

