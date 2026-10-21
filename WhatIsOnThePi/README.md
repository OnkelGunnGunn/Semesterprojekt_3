# 3.Semesterprojekt Turtlebot3

Dette Repository indeholder alle de ROS2 filer som bruges i semesterprojektet Turtlebot3 kurset på 3. semester robot diplomingeniør.

## Install ROS 2 Packages on Raspberry Pi for the TurtleBot3

To install ROS 2 and the ROS 2 Packages on Raspberry Pi for the TurtleBot3, follow these guides: \
https://emanual.robotis.com/docs/en/platform/turtlebot3/sbc_setup/#sbc-setup \
https://emanual.robotis.com/docs/en/platform/turtlebot3/bringup/#bringup

## Build and run the rb3_ws workspace

- Run the following command:

```
cd ~/rb3_ws/src
colcon build --symlink-install
source ~/rb3_ws/install/setup.bash
ros2 run mqtt_2_cmd_pkg mqtt_2_cmdvel_node
```

## Build and run the code_ws workspace

- Run the following command:

```
cd  ~/code_ws/src
colcon build --symlink-install
source install/local_setup.bash
ros2 run rb3_package rb3_node
ros2 run rb3_package_cpp rb3_node_cpp
```

## Authors and acknowledgment
Show your appreciation to those who have contributed to the project.



## Project status
Ready for autumn 2025.
