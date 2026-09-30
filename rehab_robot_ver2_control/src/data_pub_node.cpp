// #include <rclcpp/rclcpp.hpp>
// #include <std_msgs/msg/float32.hpp>
// #include <linux/can.h>
// #include <linux/can/raw.h>
// #include <net/if.h>
// #include <sys/ioctl.h>
// #include <sys/socket.h>
// #include <poll.h>
// #include <cstring>
// #include <iostream>
// #include <unistd.h>

// #define KV 170.0  // Giá trị KV của động cơ

// class DataPubNode : public rclcpp::Node {
// public:
//     DataPubNode() : Node("data_pub") {
//         // Publisher cho Node ID 0
//         position_pub_0_ = this->create_publisher<std_msgs::msg::Float32>("motor/position_0", 10);
//         velocity_pub_0_ = this->create_publisher<std_msgs::msg::Float32>("motor/velocity_0", 10);
//         iq_setpoint_pub_0_ = this->create_publisher<std_msgs::msg::Float32>("motor/iq_setpoint_0", 10);
//         iq_measured_pub_0_ = this->create_publisher<std_msgs::msg::Float32>("motor/iq_measured_0", 10);
//         torque_setpoint_pub_0_ = this->create_publisher<std_msgs::msg::Float32>("motor/torque_setpoint_0", 10);
//         torque_actual_pub_0_ = this->create_publisher<std_msgs::msg::Float32>("motor/torque_actual_0", 10);

//         // Publisher cho Node ID 1
//         position_pub_1_ = this->create_publisher<std_msgs::msg::Float32>("motor/position_1", 10);
//         velocity_pub_1_ = this->create_publisher<std_msgs::msg::Float32>("motor/velocity_1", 10);
//         iq_setpoint_pub_1_ = this->create_publisher<std_msgs::msg::Float32>("motor/iq_setpoint_1", 10);
//         iq_measured_pub_1_ = this->create_publisher<std_msgs::msg::Float32>("motor/iq_measured_1", 10);
//         torque_setpoint_pub_1_ = this->create_publisher<std_msgs::msg::Float32>("motor/torque_setpoint_1", 10);
//         torque_actual_pub_1_ = this->create_publisher<std_msgs::msg::Float32>("motor/torque_actual_1", 10);

//         // Publisher cho Node ID 2
//         position_pub_2_ = this->create_publisher<std_msgs::msg::Float32>("motor/position_2", 10);
//         velocity_pub_2_ = this->create_publisher<std_msgs::msg::Float32>("motor/velocity_2", 10);
//         iq_setpoint_pub_2_ = this->create_publisher<std_msgs::msg::Float32>("motor/iq_setpoint_2", 10);
//         iq_measured_pub_2_ = this->create_publisher<std_msgs::msg::Float32>("motor/iq_measured_2", 10);
//         torque_setpoint_pub_2_ = this->create_publisher<std_msgs::msg::Float32>("motor/torque_setpoint_2", 10);
//         torque_actual_pub_2_ = this->create_publisher<std_msgs::msg::Float32>("motor/torque_actual_2", 10);

//         socket_can_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
//         if (socket_can_ < 0) {
//             RCLCPP_ERROR(this->get_logger(), "Failed to open CAN socket!");
//             rclcpp::shutdown();
//         }

//         struct ifreq ifr;
//         strcpy(ifr.ifr_name, "can0");
//         ioctl(socket_can_, SIOCGIFINDEX, &ifr);

//         struct sockaddr_can addr;
//         addr.can_family = AF_CAN;
//         addr.can_ifindex = ifr.ifr_ifindex;
//         if (bind(socket_can_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
//             RCLCPP_ERROR(this->get_logger(), "Failed to bind CAN socket!");
//             rclcpp::shutdown();
//         }

//         // Chạy vòng lặp đọc dữ liệu ngay khi có (poll)
//         thread_ = std::thread(&DataPubNode::read_can_data, this);
//         RCLCPP_INFO(this->get_logger(), "DataPubNode started for Node ID 0, 1, and 2");
//     }

//     ~DataPubNode() {
//         if (thread_.joinable()) {
//             thread_.join();
//         }
//         close(socket_can_);
//     }

// private:
//     int socket_can_;
//     std::thread thread_;

//     // Publisher cho Node ID 0
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr position_pub_0_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr velocity_pub_0_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr iq_setpoint_pub_0_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr iq_measured_pub_0_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr torque_setpoint_pub_0_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr torque_actual_pub_0_;

//     // Publisher cho Node ID 1
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr position_pub_1_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr velocity_pub_1_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr iq_setpoint_pub_1_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr iq_measured_pub_1_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr torque_setpoint_pub_1_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr torque_actual_pub_1_;

//     // Publisher cho Node ID 2
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr position_pub_2_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr velocity_pub_2_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr iq_setpoint_pub_2_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr iq_measured_pub_2_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr torque_setpoint_pub_2_;
//     rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr torque_actual_pub_2_;

//     float bytes_to_float(const uint8_t* bytes) {
//         float value;
//         std::memcpy(&value, bytes, sizeof(float));
//         return value;
//     }

//     void read_can_data() {
//         struct pollfd pfd;
//         pfd.fd = socket_can_;
//         pfd.events = POLLIN;

//         while (rclcpp::ok()) {
//             int ret = poll(&pfd, 1, 1); // Timeout 1ms để tăng tần suất
//             if (ret > 0 && (pfd.revents & POLLIN)) {
//                 struct can_frame frame;
//                 int nbytes = read(socket_can_, &frame, sizeof(struct can_frame));
//                 if (nbytes > 0) {
//                     process_can_data(frame);
//                 }
//             }
//         }
//     }

//     void process_can_data(const struct can_frame& frame) {
//         // Kiểm tra độ dài frame (trừ lệnh gửi torque: 0x0E cho Node 0, 0x2E cho Node 1, 0x4E cho Node 2)
//         if (frame.can_dlc != 8 && frame.can_id != ((0 << 5) | 0x0E) && frame.can_id != ((1 << 5) | 0x0E) && frame.can_id != ((2 << 5) | 0x0E)) {
//             RCLCPP_WARN(this->get_logger(), "Invalid CAN frame size or ID: 0x%03x", frame.can_id);
//             return;
//         }

//         int node_id = frame.can_id >> 5; // Lấy Node ID
//         int command_id = frame.can_id & 0x1F; // Lấy Command ID

//         if (command_id == 0x09) { // Get_Encoder_Estimates
//             float position = bytes_to_float(frame.data);
//             float velocity = bytes_to_float(frame.data + 4);
//             if (node_id == 0) { // Node ID 0 (CAN ID = 0x09)
//                 publish_float(position_pub_0_, position);
//                 publish_float(velocity_pub_0_, velocity);
//                 RCLCPP_INFO(this->get_logger(), "Node 0: Pos = %.3f turns, Vel = %.3f turns/s", position, velocity);
//             } else if (node_id == 1) { // Node ID 1 (CAN ID = 0x29)
//                 publish_float(position_pub_1_, position);
//                 publish_float(velocity_pub_1_, velocity);
//                 RCLCPP_INFO(this->get_logger(), "Node 1: Pos = %.3f turns, Vel = %.3f turns/s", position, velocity);
//             } else if (node_id == 2) { // Node ID 2 (CAN ID = 0x49)
//                 publish_float(position_pub_2_, position);
//                 publish_float(velocity_pub_2_, velocity);
//                 RCLCPP_INFO(this->get_logger(), "Node 2: Pos = %.3f turns, Vel = %.3f turns/s", position, velocity);
//             }
//         } 
//         else if (command_id == 0x14) { // Get_Iq
//             float iq_setpoint = bytes_to_float(frame.data);
//             float iq_measured = bytes_to_float(frame.data + 4);
//             float torque_setpoint = (iq_setpoint * 8.27) / KV;
//             float torque_actual = (iq_measured * 8.27) / KV;

//             if (node_id == 0) { // Node ID 0 (CAN ID = 0x14)
//                 publish_float(iq_setpoint_pub_0_, iq_setpoint);
//                 publish_float(iq_measured_pub_0_, iq_measured);
//                 publish_float(torque_setpoint_pub_0_, torque_setpoint);
//                 publish_float(torque_actual_pub_0_, torque_actual);
//                 RCLCPP_INFO(this->get_logger(), "Node 0: Iq_set = %.3f A, Iq_meas = %.3f A, Torque_set = %.3f Nm, Torque_act = %.3f Nm", 
//                             iq_setpoint, iq_measured, torque_setpoint, torque_actual);
//             } else if (node_id == 1) { // Node ID 1 (CAN ID = 0x34)
//                 publish_float(iq_setpoint_pub_1_, iq_setpoint);
//                 publish_float(iq_measured_pub_1_, iq_measured);
//                 publish_float(torque_setpoint_pub_1_, torque_setpoint);
//                 publish_float(torque_actual_pub_1_, torque_actual);
//                 RCLCPP_INFO(this->get_logger(), "Node 1: Iq_set = %.3f A, Iq_meas = %.3f A, Torque_set = %.3f Nm, Torque_act = %.3f Nm", 
//                             iq_setpoint, iq_measured, torque_setpoint, torque_actual);
//             } else if (node_id == 2) { // Node ID 2 (CAN ID = 0x54)
//                 publish_float(iq_setpoint_pub_2_, iq_setpoint);
//                 publish_float(iq_measured_pub_2_, iq_measured);
//                 publish_float(torque_setpoint_pub_2_, torque_setpoint);
//                 publish_float(torque_actual_pub_2_, torque_actual);
//                 RCLCPP_INFO(this->get_logger(), "Node 2: Iq_set = %.3f A, Iq_meas = %.3f A, Torque_set = %.3f Nm, Torque_act = %.3f Nm", 
//                             iq_setpoint, iq_measured, torque_setpoint, torque_actual);
//             }
//         }
//     }

//     void publish_float(const rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr& pub, float value) {
//         auto msg = std_msgs::msg::Float32();
//         msg.data = value;
//         pub->publish(msg);
//     }
// };

// int main(int argc, char** argv) {
//     rclcpp::init(argc, argv);
//     rclcpp::spin(std::make_shared<DataPubNode>());
//     rclcpp::shutdown();
//     return 0;
// }

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float32.hpp>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <poll.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <map>

#define KV 170.0  // Giá trị KV của động cơ

class DataPubNode : public rclcpp::Node {
public:
    DataPubNode() : Node("data_pub") {
        std::vector<int> node_ids = {0, 1, 2};

        for (int id : node_ids) {
            // Position & velocity
            position_pub_[id] = this->create_publisher<std_msgs::msg::Float32>("motor/position_" + std::to_string(id), 10);
            velocity_pub_[id] = this->create_publisher<std_msgs::msg::Float32>("motor/velocity_" + std::to_string(id), 10);
            // Iq
            iq_setpoint_pub_[id] = this->create_publisher<std_msgs::msg::Float32>("motor/iq_setpoint_" + std::to_string(id), 10);
            iq_measured_pub_[id] = this->create_publisher<std_msgs::msg::Float32>("motor/iq_measured_" + std::to_string(id), 10);
            // Torque
            torque_setpoint_pub_[id] = this->create_publisher<std_msgs::msg::Float32>("motor/torque_setpoint_" + std::to_string(id), 10);
            torque_actual_pub_[id] = this->create_publisher<std_msgs::msg::Float32>("motor/torque_actual_" + std::to_string(id), 10);

            // Initialize filtered torque
            filtered_torque_[id] = 0.0;
        }

        // Mở socket CAN
        socket_can_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (socket_can_ < 0) {
            RCLCPP_ERROR(this->get_logger(), "Failed to open CAN socket!");
            rclcpp::shutdown();
        }

        struct ifreq ifr;
        strcpy(ifr.ifr_name, "can0");
        ioctl(socket_can_, SIOCGIFINDEX, &ifr);

        struct sockaddr_can addr;
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;
        if (bind(socket_can_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            RCLCPP_ERROR(this->get_logger(), "Failed to bind CAN socket!");
            rclcpp::shutdown();
        }

        thread_ = std::thread(&DataPubNode::read_can_data, this);
        RCLCPP_INFO(this->get_logger(), "DataPubNode started for Node IDs 0,1,2");
    }

    ~DataPubNode() {
        if (thread_.joinable()) thread_.join();
        close(socket_can_);
    }

private:
    int socket_can_;
    std::thread thread_;

    std::map<int, rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr> position_pub_;
    std::map<int, rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr> velocity_pub_;
    std::map<int, rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr> iq_setpoint_pub_;
    std::map<int, rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr> iq_measured_pub_;
    std::map<int, rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr> torque_setpoint_pub_;
    std::map<int, rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr> torque_actual_pub_;

    std::map<int, double> filtered_torque_;  // Low pass filter for torque_actual

    const double alpha_ = 0.1;  // Low pass filter coefficient
    const double GEARBOX_RATIO = 15.0;  // Gearbox reduction ratio for node 1

    float bytes_to_float(const uint8_t* bytes) {
        float value;
        std::memcpy(&value, bytes, sizeof(float));
        return value;
    }

    void read_can_data() {
        struct pollfd pfd;
        pfd.fd = socket_can_;
        pfd.events = POLLIN;

        while (rclcpp::ok()) {
            int ret = poll(&pfd, 1, 1); // timeout 1ms
            if (ret > 0 && (pfd.revents & POLLIN)) {
                struct can_frame frame;
                int nbytes = read(socket_can_, &frame, sizeof(struct can_frame));
                if (nbytes > 0) process_can_data(frame);
            }
        }
    }

    void process_can_data(const struct can_frame& frame) {
        if (frame.can_dlc != 8) {
            RCLCPP_WARN(this->get_logger(), "Invalid CAN frame size: 0x%03x, dlc=%d", frame.can_id, frame.can_dlc);
            return;
        }

        int node_id = frame.can_id >> 5;
        int command_id = frame.can_id & 0x1F;

        switch (command_id) {
            case 0x09: { // Get_Encoder_Estimates
                float pos = bytes_to_float(frame.data);
                float vel = bytes_to_float(frame.data + 4);
                publish_position_velocity(node_id, pos, vel);
                break;
            }
            case 0x14: { // Get_Iq
                float iq_set = bytes_to_float(frame.data);
                float iq_meas = bytes_to_float(frame.data + 4);
                float torque_set = (iq_set * 8.27) / KV;
                float torque_act = (iq_meas * 8.27) / KV;
                publish_torque(node_id, iq_set, iq_meas, torque_set, torque_act);
                break;
            }
            default:
                break;
        }
    }

    void publish_position_velocity(int node_id, float pos, float vel) {
        if (node_id == 1) {
            pos /= GEARBOX_RATIO;
            vel /= GEARBOX_RATIO;
        }
        if (position_pub_.count(node_id)) {
            publish_float(position_pub_[node_id], pos);
            publish_float(velocity_pub_[node_id], vel);
        }
    }

    void publish_torque(int node_id, float iq_set, float iq_meas, float torque_set, float torque_act) {
        if (iq_setpoint_pub_.count(node_id)) {
            publish_float(iq_setpoint_pub_[node_id], iq_set);
            publish_float(iq_measured_pub_[node_id], iq_meas);

            if (node_id == 1) {
                torque_set *= GEARBOX_RATIO;
            }
            publish_float(torque_setpoint_pub_[node_id], torque_set);

            // Apply low pass filter to torque_actual
            filtered_torque_[node_id] = alpha_ * torque_act + (1.0 - alpha_) * filtered_torque_[node_id];

            double published_torque = filtered_torque_[node_id];
            if (node_id == 1) {
                published_torque *= GEARBOX_RATIO;
            }
            publish_float(torque_actual_pub_[node_id], published_torque);
        }
    }

    void publish_float(const rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr& pub, float value) {
        auto msg = std_msgs::msg::Float32();
        msg.data = value;
        pub->publish(msg);
    }
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DataPubNode>());
    rclcpp::shutdown();
    return 0;
}


