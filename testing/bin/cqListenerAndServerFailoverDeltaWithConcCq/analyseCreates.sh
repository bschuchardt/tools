#!/bin/sh
# this works with cqListenerAndServerFailoverDeltaWithConcCq

grep "put this key " vm_*_feed1_*.log >put.txt
#[info 2009/12/10 09:45:35.806 PST <vm_10_thr_10_feed1_frodo_18060> tid=0x4f] put this key Object_-1 in the map for putAll

grep "put this key:" vm_*_feed1_*.log >>put.txt
#[info 2009/12/10 09:48:00.559 PST <vm_10_thr_11_feed1_frodo_18060> tid=0xd4] put this key: Object_931 in testRegion3

cat put.txt | cut -d " " -f 10 >putKeys.txt
sort putKeys.txt >putKeys.srt

grep -C 2 "getBaseOperation(): CREATE" vm_*edge*.log | grep "getKey()" >afterCreate.txt
cat afterCreate.txt | cut -d " " -f 7 | sort >afterCreateKeys.srt 

# in past failure there were no events at all for some of the keys.
# since there are multiple regions, that's good because we can use uniq
# to simplify the files and do a diff
uniq putKeys.srt >putKeysUniq.srt
uniq afterCreateKeys.srt >afterCreateKeysUniq.srt

diff putKeysUniq.srt afterCreateKeysUniq.srt | grep Object \
  | cut -d " " -f 2 > missingCreates.txt

