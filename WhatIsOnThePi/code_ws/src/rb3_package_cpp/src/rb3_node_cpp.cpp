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

    phase_start_ = this->get_clock()->now();
    timer_ = this->create_wall_timer(
      100ms,
      std::bind(&RB3_cpp_publisher::publish_vel, this));
  }

private:
  enum class MotionPhase { Forward, Turn };

  void publish_vel()
  {
    const auto now = this->get_clock()->now();
    const double elapsed = (now - phase_start_).seconds();

    const double forward_duration = side_length_ / linear_speed_;
    const double turn_duration = (std::acos(-1.0) / 2.0) / angular_speed_;

    geometry_msgs::msg::TwistStamped msg;
    msg.header.stamp = now;
    msg.header.frame_id = "base_link";

    msg.twist.linear.y = 0.0;
    msg.twist.linear.z = 0.0;
    msg.twist.angular.x = 0.0;
    msg.twist.angular.y = 0.0;

    if (phase_ == MotionPhase::Forward) {
      msg.twist.linear.x = linear_speed_;
      msg.twist.angular.z = 0.0;

      if (elapsed >= forward_duration) {
        phase_ = MotionPhase::Turn;
        phase_start_ = now;
      }
    } else {
      msg.twist.linear.x = 0.0;
      msg.twist.angular.z = angular_speed_;

      if (elapsed >= turn_duration) {
        phase_ = MotionPhase::Forward;
        phase_start_ = now;
      }
    }

    publisher_->publish(msg);
  }

  MotionPhase phase_ = MotionPhase::Forward;
  rclcpp::Time phase_start_;
  double linear_speed_ = 0.15;
  double angular_speed_ = 0.8;
  double side_length_ = 1.0;

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
