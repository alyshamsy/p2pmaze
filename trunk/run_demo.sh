#!/bin/bash

MASTER_CONFIG_FILE=config_demo.ini
MASTER_PORT=7878
SERVER1_PORT=3000
SERVER2_PORT=3001
HOST=localhost

#color()
#{
#	echo -n -e "\33[22;31m"
#}
#no_color()
#{
#	tput sgr0
#}

#color
echo "Compiling game sources ..."
#no_color
make
make -C src/gamemonitor

#color
echo "Runnig master ..."
#no_color
xterm -e "./superpeer $MASTER_CONFIG_FILE $MASTER_PORT /tmp/master.log" &
sleep 1

#color
echo "Runnig server1 ..."
#no_color
xterm -e "./peer $HOST:$MASTER_PORT $HOST:$SERVER1_PORT " &
sleep 0.2

#color
echo "Runnig server2 ..."
#no_color
xterm -e "./peer $HOST:$MASTER_PORT $HOST:$SERVER2_PORT " &
sleep 1

#color
echo "Runnig 100 clients ..."
#no_color
for (( i=1; i<=50; i++ )); do ./client $HOST:$SERVER1_PORT > /dev/null & echo -n $i" " ; sleep 0.03; done
for (( i=51; i<=100; i++ )); do ./client $HOST:$SERVER2_PORT > /dev/null & echo -n $i" " ; sleep 0.03; done
echo

#color
echo "Runnig game monitor ..."
#no_color
src/gamemonitor/gamemonitor /tmp/master.log  &
sleep 2

#color
echo "Runnig client with GUI ..."
#no_color
./client --gui $HOST:$SERVER1_PORT

#color
echo "Terminating ..."
#no_color
echo "Killing master ..."
killall -s SIGKILL master
echo "Servers will soon disconnect ..."
echo "Client will timeout and disconnect ..."
echo "Killing the game monitor ..."
killall -s SIGKILL gamemonitor

#color
echo "Finished"
#no_color
