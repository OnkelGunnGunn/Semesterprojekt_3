# Semesterprojekt_3


## How to run it on the robot

1. Turn on a wifi hotspot on a phone (by default Mathias' wifi, because this is what is written in the PI's config file)
2. Turn on the robot
3. run ```sudo arp-scan -l```
4. on line nr. 21 in the file **run_on_robot.cpp** or **run_on_robot_with_compile.cpp**, change the localfilename to the file you want to test on the robot.
5. This step depends if you have changed the code since last run.
    * newly changed code: ```g++ run_on_robot.cpp -o main.exe``` & ```./main.exe```
    * Code hasn't changed since last run: ```g++ run_on_robot_with_compile.cpp -o main.exe``` & ```./main.exe```
6. Type the code, ```turtlebot```, into the terminal