#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"

using namespace std;

namespace my_namespace {
class MyComponent : public rclcpp::Node {
public:
    explicit MyComponent(const rclcpp::NodeOptions & options)
    : Node("my_component", options) {
        RCLCPP_INFO(this->get_logger(), "Component initialized successfully");
    }
};
}

RCLCPP_COMPONENTS_REGISTER_NODE(my_namespace::MyComponent)
