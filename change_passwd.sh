#!/bin/bash

if [ $# != 2 ]; then
	echo "Usage: $0 user new_passwd"
	exit -1;
fi

export USERCH=$1
export PASSWD=$2

(echo "$PASSWD"; sleep 1; echo "$PASSWD") | passwd $USERCH > /dev/null 2>&1 && exit 0

exit -1

