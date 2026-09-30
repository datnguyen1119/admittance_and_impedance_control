#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <vector>

class MotorStatePublisher : public rclcpp::Node {
public:
    MotorStatePublisher() : Node("motor_state_publisher") {

        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();

        this->declare_parameter("motor_node_ids", std::vector<int64_t>{0, 1, 2});
        this->declare_parameter("node1_gear_ratio", 15.0);
        this->declare_parameter("publish_rate", 10.0);

        motor_node_ids_ = this->get_parameter("motor_node_ids").as_integer_array();
        node1_gear_ratio_ = this->get_parameter("node1_gear_ratio").as_double();
        double publish_rate = this->get_parameter("publish_rate").as_double();

        // Tên khớp tương ứng với motor ID
        joint_names_ = {"base_joint", "shoulder_joint", "elbow_joint"};

        // Tạo publishers cho từng khớp
        for (const auto &name : joint_names_) {
            pos_pubs_[name] = this->create_publisher<std_msgs::msg::Float64>("/" + name + "/m_pos", 10);
            vel_pubs_[name] = this->create_publisher<std_msgs::msg::Float64>("/" + name + "/m_vel", 10);
        }

        request_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate)),
            std::bind(&MotorStatePublisher::request_encoder_data, this));

        publish_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate) + 10),
            std::bind(&MotorStatePublisher::publish_joint_state_topics, this));

        RCLCPP_INFO(this->get_logger(), "Publishing separate joint state topics at %.1f Hz", publish_rate);
    }

private:
    void request_encoder_data() {
        odrive_->ReadEncoder();
    }

    void publish_joint_state_topics() {

        const double REV_TO_RAD = M_PI / 4.0;

        for (size_t i = 0; i < motor_node_ids_.size(); i++) {

            int node_id = static_cast<int>(motor_node_ids_[i]);
            std::string joint = joint_names_[i];

            float turns = 0.0;
            float vel_turns = 0.0;
            odrive_->get_encoder_data(node_id, turns, vel_turns);

            // Convert turns → rad
            float angle_rad, vel_rad;

            if (node_id == 1) {
                angle_rad = (turns / node1_gear_ratio_) * REV_TO_RAD;
                vel_rad = (vel_turns / node1_gear_ratio_) * REV_TO_RAD;
            } else {
                angle_rad = turns * REV_TO_RAD;
                vel_rad = vel_turns * REV_TO_RAD;
            }

            // Publish ra từng topic riêng
            std_msgs::msg::Float64 pos_msg;
            pos_msg.data = angle_rad;
            pos_pubs_[joint]->publish(pos_msg);

            std_msgs::msg::Float64 vel_msg;
            vel_msg.data = vel_rad;
            vel_pubs_[joint]->publish(vel_msg);
        }
    }

    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    rclcpp::TimerBase::SharedPtr request_timer_;
    rclcpp::TimerBase::SharedPtr publish_timer_;

    std::vector<int64_t> motor_node_ids_;
    double node1_gear_ratio_;

    std::vector<std::string> joint_names_;
    std::unordered_map<std::string, rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr> pos_pubs_;
    std::unordered_map<std::string, rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr> vel_pubs_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MotorStatePublisher>());
    rclcpp::shutdown();
    return 0;
}
