#!/bin/bash

echo "compile client and server"
gcc -c seeWhat.c && gcc seeWhat.c -o seeWhat -lm
gcc -c timeServer.c && gcc timeServer.c -o timeServer -lm
gcc -c showResult.c && gcc showResult.c -o showResult
echo "start server"
gnome-terminal -x  ./timeServer 5 5 "mainFifo"

echo "start new client 1"
gnome-terminal -x ./seeWhat "mainFifo"
echo "start new client 2"
gnome-terminal -x ./seeWhat "mainFifo"
echo "start new client 3"
gnome-terminal -x ./seeWhat "mainFifo"
sleep 10
echo "start showResult"
gnome-terminal -x ./showResult






