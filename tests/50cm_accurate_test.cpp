#include <chrono>
#include <cmath>
#include <functional>
#include <memory>

#include "geometry_msgs/msg/twist_stamped.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class RB3_cpp_publisher : public rclcpp::Node
{
public:
  RB3_cpp_publisher()
  : Node("rb3_node_cpp")
  {
    publisher_ = this->create_publisher<geometry_msgs::msg::TwistStamped>("cmd_vel", 10);

    start_time_ = this->get_clock()->now();
    phase_start_ = start_time_;
    timer_ = this->create_wall_timer(
      100ms,
      std::bind(&RB3_cpp_publisher::publish_vel, this));
  }

private:
  enum class MotionPhase { Forward, Turn };

  void publish_vel()
  {
    const auto now = this->get_clock()->now();
    // const double forward_duration = side_length_ / linear_speed_;

    geometry_msgs::msg::TwistStamped msg;
    msg.header.stamp = now;
    msg.header.frame_id = "base_link";

    // The robot cannot move in these directions so they are set to 0
    msg.twist.linear.y = 0.0;
    msg.twist.linear.z = 0.0;
    msg.twist.angular.x = 0.0;
    msg.twist.angular.y = 0.0;

    if ((now - start_time_).seconds() >= 2.5) {
      msg.twist.linear.x = 0.0;
      msg.twist.angular.z = 0.0;
      publisher_->publish(msg);
      rclcpp::shutdown();
      return;
    }

    if (phase_ == MotionPhase::Forward) {
      msg.twist.linear.x = linear_speed_;
      msg.twist.angular.z = 0.0;
    }
    phase_ = MotionPhase::Forward;

    publisher_->publish(msg);
  }

  MotionPhase phase_ = MotionPhase::Forward;
  rclcpp::Time start_time_;
  rclcpp::Time phase_start_;
  double linear_speed_ = 0.20; // 0.05, 0.10, 0.20
  double angular_speed_ = 0.0;
  //double side_length_ = 0.5;

  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RB3_cpp_publisher>());
  rclcpp::shutdown();
  return 0;
}
