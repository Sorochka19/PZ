#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include "lab2_interfaces/srv/reset_filter.hpp"
#include "lab2_interfaces/action/process_data.hpp"
#include <thread>
#include <algorithm>

using namespace std::placeholders;

class DataFilterNode : public rclcpp::Node {
public:
    using ResetFilter = lab2_interfaces::srv::ResetFilter;
    using ProcessData = lab2_interfaces::action::ProcessData;
    using GoalHandleProcessData = rclcpp_action::ServerGoalHandle<ProcessData>;

    DataFilterNode() : Node("data_filter") {
        // Завдання 3: QoS профіль transient_local, глибина 1
        rclcpp::QoS qos_profile(1);
        qos_profile.transient_local();

        // Завдання 4: Підтримка перевизначення політик QoS
        rclcpp::PublisherOptions pub_options;
        pub_options.qos_overriding_options = rclcpp::QosOverridingOptions::with_default_policies();

        // Завдання 2: "Ледача" публікація через matched_callback
        pub_options.event_callbacks.matched_callback =
            [this](rclcpp::MatchedInfo & info) {
                if (info.current_count > 0 && !sub_) {
                    RCLCPP_INFO(this->get_logger(), "Слухач підключився, створюємо підписку.");
                    sub_ = this->create_subscription<std_msgs::msg::String>(
                        "input_topic", 10, std::bind(&DataFilterNode::topic_callback, this, _1));
                } else if (info.current_count == 0 && sub_) {
                    RCLCPP_INFO(this->get_logger(), "Слухачі відсутні, видаляємо підписку.");
                    sub_.reset();
                }
            };

        // Завдання 1: Видавець вихідної теми uppercase_topic
        pub_ = this->create_publisher<std_msgs::msg::String>("uppercase_topic", qos_profile, pub_options);

        // Завдання 5: Асинхронний сервіс та його інтроспекція
        srv_ = this->create_service<ResetFilter>("reset_filter",
            std::bind(&DataFilterNode::handle_reset, this, _1, _2));
        srv_->configure_introspection(this->get_clock(), rclcpp::SystemDefaultsQoS(), RCL_SERVICE_INTROSPECTION_CONTENTS);

        // Завдання 6: Action-сервер для асинхронної обробки
        action_server_ = rclcpp_action::create_server<ProcessData>(
            this, "process_data",
            std::bind(&DataFilterNode::handle_goal, this, _1, _2),
            std::bind(&DataFilterNode::handle_cancel, this, _1),
            std::bind(&DataFilterNode::handle_accepted, this, _1));
    }

private:
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
    rclcpp::Service<ResetFilter>::SharedPtr srv_;
    rclcpp_action::Server<ProcessData>::SharedPtr action_server_;

    void topic_callback(const std_msgs::msg::String::SharedPtr msg) const {
        auto out_msg = std_msgs::msg::String();
        std::string text = msg->data;
        std::transform(text.begin(), text.end(), text.begin(), ::toupper);
        out_msg.data = text;
        pub_->publish(out_msg);
    }

    void handle_reset(const std::shared_ptr<ResetFilter::Request> request,
                      std::shared_ptr<ResetFilter::Response> response) {
        if (request->reset) {
            RCLCPP_INFO(this->get_logger(), "Стан фільтра скинуто.");
            response->success = true;
        }
    }

    rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID &, std::shared_ptr<const ProcessData::Goal>) {
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandleProcessData>) {
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accepted(const std::shared_ptr<GoalHandleProcessData> goal_handle) {
        // Виконання Action-цілі у окремому потоці
        std::thread{std::bind(&DataFilterNode::execute_action, this, _1), goal_handle}.detach();
    }

    void execute_action(const std::shared_ptr<GoalHandleProcessData> goal_handle) {
        auto feedback = std::make_shared<ProcessData::Feedback>();
        auto result = std::make_shared<ProcessData::Result>();
        
        for (int i = 1; i <= 10; ++i) {
            if (goal_handle->is_canceling()) {
                result->success = false;
                goal_handle->canceled(result);
                return;
            }
            feedback->progress = i * 10.0;
            goal_handle->publish_feedback(feedback);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        if (rclcpp::ok()) {
            result->success = true;
            goal_handle->succeed(result);
        }
    }
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DataFilterNode>());
    rclcpp::shutdown();
    return 0;
}
