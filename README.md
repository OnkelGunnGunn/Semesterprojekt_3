# Semesterprojekt_3


## How to run it on the robot

1. Turn on a wifi hotspot on a phone (by default Mathias' wifi, because this is what is written in the PI's config file)
2. Turn on the robot
3. run ```sudo arp-scan -l```
4. run ```g++ run_on_robot.cpp -o main.exe``` & ```./main.exe```