#!/bin/bash

MASTER_CONFIG_FILE=config_demo.ini
MASTER_PORT=6666
SERVER_PORT=5000
HOST=localhost

#color()
#{
#	echo -n -e "\33[22;31m"
#}
#no_color()
#{
#	tput sgr0
#}

python gamemonitor.py &
sleep 5

# Execute Superpeer
#color
#echo "Runnig master ..."
#no_color
xterm -e "./superpeer $MASTER_CONFIG_FILE $MASTER_PORT $HOST:50007" &
sleep 2

# Execute 100 Peers
echo "Runnig 5 peers ..."
#no_color
for (( i=0; i<5; i++ )); do 
  ./peer $HOST:$MASTER_PORT $HOST:$((SERVER_PORT+i)) & 
  echo $i; 
  sleep 0.03; 
done

sleep 10

#for (( i=1; i<=50; i++ )); do ./client $HOST:$SERVER1_PORT > /dev/null & echo -n $i" " ; sleep 0.03; done
#for (( i=51; i<=100; i++ )); do ./client $HOST:$SERVER1_PORT > /dev/null & echo -n $i" " ; sleep 0.03; done
#echo "Runnig game monitor ..."
#no_color
#src/gamemonitor/gamemonitor /tmp/master.log
#sleep 2

#color
#echo "Terminating ..."
#no_color
#echo "Killing master ..."
#killall -s SIGKILL superpeer
#echo "Servers will soon disconnect ..."
#echo "Client will timeout and disconnect ..."
#echo "Killing the game monitor ..."
#killall -s SIGKILL gamemonitor

#color
#echo "Finished"
#no_color
