import os

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch_ros.actions import Node
from launch.launch_description_sources import PythonLaunchDescriptionSource

from ament_index_python.packages import get_package_share_directory

def generate_launch_description():

    tb3_pkg_path = os.path.join(get_package_share_directory('turtlebot3_bringup'))

    turtle_bot = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(tb3_pkg_path,'launch','robot.launch.py')])
    )

    mqtt = Node(
       package='mqtt_2_cmd_pkg',
       executable='mqtt_2_cmdvel_node',
       name='mqtt_2_cmdvel_node'
    )

    return LaunchDescription([
        mqtt,
        turtle_bot
    ])


