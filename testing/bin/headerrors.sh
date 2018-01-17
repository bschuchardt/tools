#!/bin/bash
for x in */errors.txt; do
echo "-----------------------------------------------------------------------------------------------------------------------------------------------"
grep `dirname $x` oneliner.txt
head $x
echo ""
echo ""
echo ""
done
