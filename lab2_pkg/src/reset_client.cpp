#include <rclcpp/rclcpp.hpp>
#include "lab2_interfaces/srv/reset_filter.hpp"
#include <chrono>

using namespace std::chrono_literals;

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = rclcpp::Node::make_shared("reset_client");
    auto client = node->create_client<lab2_interfaces::srv::ResetFilter>("reset_filter");

    // Очікування доступності сервера перед відправкою запиту
    while (!client->wait_for_service(1s)) {
        if (!rclcpp::ok()) {
            RCLCPP_ERROR(node->get_logger(), "Роботу клієнта перервано.");
            return 0;
        }
        RCLCPP_INFO(node->get_logger(), "Очікування доступності сервісу...");
    }

    auto request = std::make_shared<lab2_interfaces::srv::ResetFilter::Request>();
    request->reset = true;
    
    auto result_future = client->async_send_request(request);
    
    if (rclcpp::spin_until_future_complete(node, result_future, 3s) == rclcpp::FutureReturnCode::SUCCESS) {
        RCLCPP_INFO(node->get_logger(), "Відповідь сервера: %d", result_future.get()->success);
    } else {
        RCLCPP_ERROR(node->get_logger(), "Таймаут очікування відповіді сервісу.");
    }
    
    rclcpp::shutdown();
    return 0;
}
