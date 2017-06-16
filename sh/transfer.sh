#!/bin/sh
archive=${1:-pc.zip}
destdir=devel/`filename $PWD`
remotehost=ent

echo "finding changed files"
files=`svn status | grep ^[A,M] | cut -c8-`
rm -f $archive

echo "archiving changed files"
zip -q $archive $files
count=`echo $files | wc -w`

echo "transfering changed files archive to ${remotehost}"
scp ${archive} ${remotehost}:$destdir/$archive

echo "unpacking archive on ${remotehost}
ssh ent cd ${destdir}\; unzip -ao ${archive}

echo "${count} files transfered to ${destdir} on ${remotehost}"
