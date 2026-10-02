#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"

using namespace std;
using rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface;

class LCNode : public rclcpp_lifecycle::LifecycleNode {
public:
    explicit LCNode(const string & node_name)
    : rclcpp_lifecycle::LifecycleNode(node_name) {}

    LifecycleNodeInterface::CallbackReturn on_configure(const rclcpp_lifecycle::State &) {
        RCLCPP_INFO(this->get_logger(), "State: Configured");
        return LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    LifecycleNodeInterface::CallbackReturn on_activate(const rclcpp_lifecycle::State &) {
        RCLCPP_INFO(this->get_logger(), "State: Activated");
        return LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    rclcpp::executors::SingleThreadedExecutor executor;
    
    auto node1 = make_shared<LCNode>("lc_node_1");
    auto node2 = make_shared<LCNode>("lc_node_2");
    
    executor.add_node(node1->get_node_base_interface());
    executor.add_node(node2->get_node_base_interface());
    
    executor.spin();
    rclcpp::shutdown();
    return 0;
}
