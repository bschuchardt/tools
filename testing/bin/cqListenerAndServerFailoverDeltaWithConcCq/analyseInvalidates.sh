#!/bin/sh
# this works with cqListenerAndServerFailoverDeltaWithConcCq
grep "Invalidated Object" vm_*_feed1_*.log >invalidated.txt
cat invalidated.txt | cut -d " " -f 8 >invalidatedKeys.txt
sort invalidatedKeys.txt >invalidatedKeys.srt
grep -C 2 "getBaseOperation(): INVALIDATE" vm_*edge*.log | grep "getKey()" >afterInvalidate.txt
cat afterInvalidate.txt | cut -d " " -f 7 | sort >afterInvalidateKeys.srt 
uniq afterInvalidateKeys.srt >afterInvalidateKeysUniq.srt
diff invalidatedKeys.srt afterInvalidateKeysUniq.srt | grep Object > missingInvalidations.txt

