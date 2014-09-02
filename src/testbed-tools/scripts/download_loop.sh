#!/bin/sh

while : ; do
	date
	wget http://10.10.2.254/debian1.iso -O /dev/null
	date
	wget http://10.10.2.254/debian2.iso -O /dev/null
	date
	wget http://10.10.2.254/debian3.iso -O /dev/null
done
