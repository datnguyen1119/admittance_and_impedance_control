// #pragma once

// #include <rclcpp/rclcpp.hpp>
// #include <kdl_parser/kdl_parser.hpp>
// #include <kdl/tree.hpp>
// #include <kdl/chain.hpp>
// #include <urdf/model.h>
// #include <iostream>
// #include <kdl/chainfksolverpos_recursive.hpp>
// #include <kdl/chainiksolverpos_lma.hpp>
// #include <kdl/jntarray.hpp>
// #include <kdl/frames.hpp>

// #include "rviz_custom_plugins/msg/kinematics_request.hpp"


// class KinematicsNode : public rclcpp::Node
// {
// public:
//     KinematicsNode() : Node("kinematics_node")
//     {
//         std::string urdf_path = this->declare_parameter<std::string>(
//             "urdf_path", "/home/dat/Workspace/rehab_robot_ver2/src/rehab_robot_ver2/urdf/rehab_robot_ver2.urdf");

//         urdf::Model model;
//         if (!model.initFile(urdf_path)) {
//             RCLCPP_ERROR(this->get_logger(), "Không thể load URDF file: %s", urdf_path.c_str());
//             return;
//         }

//         // Tạo KDL Tree từ URDF
//         KDL::Tree kdl_tree;
//         if (!kdl_parser::treeFromUrdfModel(model, kdl_tree)) {
//             RCLCPP_ERROR(this->get_logger(), "Không thể chuyển URDF thành KDL Tree!");
//             return;
//         }

//         RCLCPP_INFO(this->get_logger(), "KDL Tree được tạo thành công!");
//         RCLCPP_INFO(this->get_logger(), "Số nhánh (segments): %d", kdl_tree.getNrOfSegments());

//         // Rút chain từ base_link tới fake_hand_link
//         rclcpp::Subscription<rviz_custom_plugins::msg::KinematicsFKRequest>::SharedPtr subscription_;
//         KDL::Chain kdl_chain;
//         if (!kdl_tree.getChain("base_link", "end_effector", kdl_chain)) {
//             RCLCPP_ERROR(this->get_logger(), "Không thể tạo chain từ base_link tới lower_arm_link!");
//             return;
//         }

//         RCLCPP_INFO(this->get_logger(), "KDL Chain được tạo thành công!");
//         RCLCPP_INFO(this->get_logger(), "Số khâu (segments): %d", kdl_chain.getNrOfSegments());
//         RCLCPP_INFO(this->get_logger(), "Số khớp (joints): %d", kdl_chain.getNrOfJoints());

//         //Gọi hàm tính FK
//         computeForwardKinematics(kdl_chain);
//         computeInverseKinematics(kdl_chain);
//     }
// private:
//     void computeForwardKinematics(const KDL::Chain& chain)
//     {
//         KDL::ChainFkSolverPos_recursive fk_solver(chain);
    
//         // Giả sử tất cả khớp = 0 rad
//         KDL::JntArray joint_positions(chain.getNrOfJoints());
//         for (unsigned int i = 0; i < chain.getNrOfJoints(); i++) {
//             joint_positions(i) = 0.0;
//         }
    
//         RCLCPP_INFO(this->get_logger(), "=== Forward Kinematics cho từng segment ===");
    
//         for (unsigned int i = 0; i < chain.getNrOfSegments(); i++) {
//             KDL::Frame frame;
//             if (fk_solver.JntToCart(joint_positions, frame, i) < 0) {
//                 RCLCPP_WARN(this->get_logger(), "Không thể tính FK cho segment %d", i);
//                 continue;
//             }
        
//             const std::string& segment_name = chain.getSegment(i).getName();
        
//             double roll, pitch, yaw;
//             frame.M.GetRPY(roll, pitch, yaw);
        
//             RCLCPP_INFO(this->get_logger(),
//                 "Segment [%s]: pos = [%.4f, %.4f, %.4f], rpy = [%.3f, %.3f, %.3f]",
//                 segment_name.c_str(),
//                 frame.p.x(), frame.p.y(), frame.p.z(),
//                 roll, pitch, yaw
//             );
//         }
    
//         // Và cuối cùng, in riêng end-effector
//         KDL::Frame end_effector_pose;
//         fk_solver.JntToCart(joint_positions, end_effector_pose);
//         double roll, pitch, yaw;
//         end_effector_pose.M.GetRPY(roll, pitch, yaw);
//         RCLCPP_INFO(this->get_logger(),
//                     "\n=== End-effector (end_effector) ===\n"
//                     "pos = [%.4f, %.4f, %.4f], rpy = [%.3f, %.3f, %.3f]",
//                     end_effector_pose.p.x(), end_effector_pose.p.y(), end_effector_pose.p.z(),
//                     roll, pitch, yaw);
//     }
    
//     // void computeInverseKinematics(const KDL::Chain& chain)
//     // {
//     //     RCLCPP_INFO(this->get_logger(), "=== Bắt đầu tính Inverse Kinematics ===");

//     //     // 1️⃣ Đặt tư thế mong muốn của end-effector
//     //     // (đơn vị: mét và radian)
//     //     KDL::Frame desired_pose(
//     //         KDL::Rotation::RPY(-0.006, -0.845, 1.423),   // Góc xoay mong muốn (Roll, Pitch, Yaw)
//     //         KDL::Vector(-0.3795, -0.0290, 0.7824)            // Vị trí mong muốn (x, y, z)
//     //     );

//     //     // 2️⃣ Khởi tạo solver IK
//     //     KDL::ChainIkSolverPos_LMA ik_solver(chain);

//     //     // 3️⃣ Khởi tạo giá trị khớp ban đầu (tất cả bằng 0)
//     //     KDL::JntArray q_init(chain.getNrOfJoints());
//     //     for (unsigned int i = 0; i < chain.getNrOfJoints(); i++) {
//     //         q_init(i) = 0.0;
//     //     }

//     //     // 4️⃣ Mảng chứa kết quả
//     //     KDL::JntArray q_result(chain.getNrOfJoints());

//     //     // 5️⃣ Thực hiện tính toán IK
//     //     int result = ik_solver.CartToJnt(q_init, desired_pose, q_result);

//     //     // 6️⃣ Kiểm tra kết quả
//     //     if (result >= 0) {
//     //         RCLCPP_INFO(this->get_logger(), "✅ Tính IK thành công!");
//     //         for (unsigned int i = 0; i < chain.getNrOfJoints(); i++) {
//     //             RCLCPP_INFO(this->get_logger(), "  Joint %d: %.4f rad", i, q_result(i));
//     //         }

//     //         // 7️⃣ Kiểm tra lại bằng FK để xác nhận kết quả
//     //         KDL::ChainFkSolverPos_recursive fk_solver(chain);
//     //         KDL::Frame achieved_pose;
//     //         fk_solver.JntToCart(q_result, achieved_pose);

//     //         double roll, pitch, yaw;
//     //         achieved_pose.M.GetRPY(roll, pitch, yaw);
//     //         RCLCPP_INFO(this->get_logger(),
//     //                     "\n=== Kiểm tra lại bằng FK ===\n"
//     //                     "pos = [%.4f, %.4f, %.4f], rpy = [%.3f, %.3f, %.3f]",
//     //                     achieved_pose.p.x(), achieved_pose.p.y(), achieved_pose.p.z(),
//     //                     roll, pitch, yaw);
//     //     } else {
//     //         RCLCPP_ERROR(this->get_logger(), "❌ IK solver không hội tụ hoặc thất bại!");
//     //     }
//     // }

//     void computeInverseKinematics(const KDL::Chain& chain)
//     {
//         RCLCPP_INFO(this->get_logger(), "=== Bắt đầu tính Inverse Kinematics ===");
    
//         // Tạo một tư thế hợp lệ (lấy từ FK của một cấu hình khớp)
//         KDL::JntArray q_true(chain.getNrOfJoints());
//         for (unsigned int i = 0; i < chain.getNrOfJoints(); i++)
//             q_true(i) = 0.2 * (i + 1);  // góc khớp thử
    
//         KDL::Frame target_pose;
//         KDL::ChainFkSolverPos_recursive fk_solver(chain);
//         fk_solver.JntToCart(q_true, target_pose);
    
//         // Giải ngược lại bằng IK
//         KDL::ChainIkSolverPos_LMA ik_solver(chain, 1e-4, 500, 1e-6);
    
//         KDL::JntArray q_init(chain.getNrOfJoints());
//         for (unsigned int i = 0; i < chain.getNrOfJoints(); i++)
//             q_init(i) = 0.0;
    
//         KDL::JntArray q_result(chain.getNrOfJoints());
//         int result = ik_solver.CartToJnt(q_init, target_pose, q_result);
    
//         if (result >= 0) {
//             RCLCPP_INFO(this->get_logger(), "✅ Tính IK thành công!");
//             for (unsigned int i = 0; i < chain.getNrOfJoints(); i++)
//                 RCLCPP_INFO(this->get_logger(), "  Joint %d: %.4f rad", i, q_result(i));
//         } else {
//             RCLCPP_ERROR(this->get_logger(), "❌ IK solver không hội tụ hoặc thất bại!");
//         }
//     }
    

    
// };

// int main(int argc, char** argv)
// {
//     rclcpp::init(argc, argv);
//     rclcpp::spin(std::make_shared<KinematicsNode>());
//     rclcpp::shutdown();
//     return 0;
// }

#include <rclcpp/rclcpp.hpp>
#include <kdl_parser/kdl_parser.hpp>
#include <kdl/tree.hpp>
#include <kdl/chain.hpp>
#include <urdf/model.h>
#include <iostream>
#include <kdl/chainfksolverpos_recursive.hpp>
#include <kdl/chainiksolverpos_lma.hpp>
#include <kdl/chainiksolvervel_pinv.hpp>
#include <kdl/chainiksolverpos_nr_jl.hpp>
#include <kdl/jntarray.hpp>
#include <kdl/frames.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <std_msgs/msg/float64.hpp>


// Include message từ plugin panel
#include "rviz_custom_plugins/msg/kinematics_fk_request.hpp"
#include "rviz_custom_plugins/msg/kinematics_fk_response.hpp"
#include "rviz_custom_plugins/msg/kinematics_ik_request.hpp"
#include "rviz_custom_plugins/msg/kinematics_ik_response.hpp"


class KinematicsNode : public rclcpp::Node
{
public:
    KinematicsNode() : Node("kinematics_node")
    {
        // Load URDF
        std::string urdf_path = this->declare_parameter<std::string>(
            "urdf_path", "/home/dat/Workspace/gravitational_compensation/src/rehab_robot_ver2/urdf/rehab_robot_ver2.urdf");

        urdf::Model model;
        if (!model.initFile(urdf_path)) {
            RCLCPP_ERROR(this->get_logger(), "Không thể load URDF file: %s", urdf_path.c_str());
            return;
        }

        // Tạo KDL Tree từ URDF
        if (!kdl_parser::treeFromUrdfModel(model, kdl_tree_)) {
            RCLCPP_ERROR(this->get_logger(), "Không thể chuyển URDF thành KDL Tree!");
            return;
        }

        // Lấy chain từ base_link → end_effector
        if (!kdl_tree_.getChain("base_link", "end_effector", kdl_chain_)) {
            RCLCPP_ERROR(this->get_logger(), "Không thể tạo chain từ base_link tới end_effector!");
            return;
        }

        RCLCPP_INFO(this->get_logger(), "✅ KDL Chain sẵn sàng (%d joints)", kdl_chain_.getNrOfJoints());

        // Lấy danh sách tên khớp từ URDF
        for (auto &segment : kdl_chain_.segments) {
            if (segment.getJoint().getType() != KDL::Joint::None)
                joint_names_.push_back(segment.getJoint().getName());
        }

        RCLCPP_INFO(this->get_logger(), "🔧 Joint names loaded:");
        for(auto &n : joint_names_)
            RCLCPP_INFO(this->get_logger(), " - %s", n.c_str());

        // Đăng ký subscriber
        subscription_ = this->create_subscription<rviz_custom_plugins::msg::KinematicsFKRequest>(
            "/ForwardKinematics/request", 10,
            std::bind(&KinematicsNode::kinematicsCallback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "📡 Đã đăng ký topic /ForwardKinematics/request");

        fk_publisher_ = this->create_publisher<rviz_custom_plugins::msg::KinematicsFKResponse>(
            "/ForwardKinematics/response", 10);
        RCLCPP_INFO(this->get_logger(), "📤 Publisher /ForwardKinematics/response ready");

        //======== IK ==========
        ik_req_sub_ = this->create_subscription<rviz_custom_plugins::msg::KinematicsIKRequest>(
            "/InverseKinematics/request", 10,
            std::bind(&KinematicsNode::handleIKRequest, this, std::placeholders::_1));
            
        ik_res_pub_ = this->create_publisher<rviz_custom_plugins::msg::KinematicsIKResponse>(
            "/InverseKinematics/response", 10);

        RCLCPP_INFO(this->get_logger(), "📡 Đã đăng ký topic /InverseKinematics/request");
        RCLCPP_INFO(this->get_logger(), "📤 Publisher /InverseKinematics/response ready");
        joint_state_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("/joint_states", 10);
        RCLCPP_INFO(this->get_logger(), "📤 Publisher /joint_states ready");

        // Sub vào giá trị encoder của từng khớp
        pos_subs_.push_back(
          this->create_subscription<std_msgs::msg::Float64>("/base_joint/m_pos", 10,
            [this](const std_msgs::msg::Float64::SharedPtr msg){
              current_joint_positions_[0] = msg->data;
            }
          )
        );

        pos_subs_.push_back(
          this->create_subscription<std_msgs::msg::Float64>("/shoulder_joint/m_pos", 10,
            [this](const std_msgs::msg::Float64::SharedPtr msg){
              current_joint_positions_[1] = msg->data;
            }
          )
        );

        pos_subs_.push_back(
          this->create_subscription<std_msgs::msg::Float64>("/elbow_joint/m_pos", 10,
            [this](const std_msgs::msg::Float64::SharedPtr msg){
              current_joint_positions_[2] = msg->data;
            }
          )
        );



    }

private:
    // vị trí hiện tại
    std::vector<double> current_joint_positions_ = {0.0, 0.0, 0.0};
    // thêm giá trị giới hạn
    std::unordered_map<std::string, std::pair<double,double>> joint_limits_ = {
        {"base_joint", {-1.57, 0.0}},
        {"shoulder_joint", {-0.4, 1.2}},
        {"elbow_joint", {-1.57, 0.0}},
    };

    // Callback khi nhận được yêu cầu từ panel
    void kinematicsCallback(const rviz_custom_plugins::msg::KinematicsFKRequest::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "📨 Nhận yêu cầu KinematicsFKRequest:");
        RCLCPP_INFO(this->get_logger(), "Frame ID: %s", msg->frame_id.c_str());

        std::ostringstream names, values;
        for (size_t i = 0; i < msg->joint_names.size(); ++i) {
            names << msg->joint_names[i] << (i < msg->joint_names.size()-1 ? ", " : "");
        }
        for (size_t i = 0; i < msg->joint_positions.size(); ++i) {
            values << msg->joint_positions[i] << (i < msg->joint_positions.size()-1 ? ", " : "");
        }

        RCLCPP_INFO(this->get_logger(), "Joint Names: [%s]", names.str().c_str());
        RCLCPP_INFO(this->get_logger(), "Joint Positions: [%s]", values.str().c_str());

        if (msg->joint_positions.size() != kdl_chain_.getNrOfJoints()) {
            RCLCPP_WARN(this->get_logger(),
                "⚠️ Số lượng joint (%zu) không khớp với chain (%d).",
                msg->joint_positions.size(), kdl_chain_.getNrOfJoints());
            return;
        }

        // Gọi hàm FK
        computeForwardKinematics(kdl_chain_, msg->joint_positions);
    }

    void computeForwardKinematics(const KDL::Chain& chain, const std::vector<double>& joint_positions_vec)
    {
        KDL::JntArray joint_positions(chain.getNrOfJoints());
        for (unsigned int i = 0; i < chain.getNrOfJoints(); i++) {
            joint_positions(i) = joint_positions_vec[i];
        }

        KDL::ChainFkSolverPos_recursive fk_solver(chain);
        KDL::Frame end_effector_pose;
        fk_solver.JntToCart(joint_positions, end_effector_pose);

        double roll, pitch, yaw;
        end_effector_pose.M.GetRPY(roll, pitch, yaw);
        RCLCPP_INFO(this->get_logger(),
                    "🦾 End-effector pose:\n"
                    "  pos = [%.4f, %.4f, %.4f]\n"
                    "  rpy = [%.3f, %.3f, %.3f]",
                    end_effector_pose.p.x(), end_effector_pose.p.y(), end_effector_pose.p.z(),
                    roll, pitch, yaw);
        // ======= PUBLISH TO PANEL =======
        if (fk_publisher_) {
            rviz_custom_plugins::msg::KinematicsFKResponse msg;
            msg.pose.position.x = end_effector_pose.p.x();
            msg.pose.position.y = end_effector_pose.p.y();
            msg.pose.position.z = end_effector_pose.p.z();
        
            // Chuyển RPY thành quaternion
            tf2::Quaternion q;
            q.setRPY(roll, pitch, yaw);
            msg.pose.orientation.x = q.x();
            msg.pose.orientation.y = q.y();
            msg.pose.orientation.z = q.z();
            msg.pose.orientation.w = q.w();
        
            fk_publisher_->publish(msg);
        }
    }

    void handleIKRequest(const rviz_custom_plugins::msg::KinematicsIKRequest::SharedPtr msg)
    {
        if (joint_names_.size() != kdl_chain_.getNrOfJoints()) {
            RCLCPP_ERROR(this->get_logger(), "❌ joint_names_ size mismatch! (%zu vs %d)",
                joint_names_.size(), kdl_chain_.getNrOfJoints());
            return;
        }else{
            RCLCPP_INFO(this->get_logger(), "✅ joint_names_ size match! (%zu vs %d)",
                joint_names_.size(), kdl_chain_.getNrOfJoints());
        }

        RCLCPP_INFO(this->get_logger(), "📨 Nhận yêu cầu IK");

        // 1. Convert ROS pose -> KDL frame
        KDL::Frame target_pose(
        KDL::Rotation::Quaternion(msg->target_pose.orientation.x,
                                  msg->target_pose.orientation.y,
                                  msg->target_pose.orientation.z,
                                  msg->target_pose.orientation.w),
        KDL::Vector(msg->target_pose.position.x,
                    msg->target_pose.position.y,
                    msg->target_pose.position.z));
        
        //debug print target pose
        double tx = msg->target_pose.position.x;
        double ty = msg->target_pose.position.y;
        double tz = msg->target_pose.position.z;
        double r, p, y;
        target_pose.M.GetRPY(r, p, y);
        RCLCPP_INFO(this->get_logger(), "Target pose: pos=[%.4f, %.4f, %.4f], rpy=[%.3f, %.3f, %.3f]", tx, ty, tz, r, p, y);

        // 2. Setup IK Solver
        KDL::ChainFkSolverPos_recursive fk_solver(kdl_chain_);
        KDL::ChainIkSolverVel_pinv ik_vel_solver(kdl_chain_);

        // Tạo mảng giới hạn từ unordered_map
        KDL::JntArray q_min(kdl_chain_.getNrOfJoints());
        KDL::JntArray q_max(kdl_chain_.getNrOfJoints());

         for (size_t i = 0; i < joint_names_.size(); ++i) {
            const auto &name = joint_names_[i];
            auto it = joint_limits_.find(name);
            if (it == joint_limits_.end()) {
                RCLCPP_ERROR(this->get_logger(), "❌ joint_limits_ missing for joint '%s' — add it to the map!", name.c_str());
                return;
            }
            q_min(i) = it->second.first;
            q_max(i) = it->second.second;
            // Log limits
            RCLCPP_INFO(this->get_logger(), "Joint %s limits: [%.4f, %.4f]", name.c_str(), q_min(i), q_max(i));
            if (q_min(i) > q_max(i)) {
                RCLCPP_ERROR(this->get_logger(), "❌ For joint %s, min > max (%.4f > %.4f). Fix limits.", name.c_str(), q_min(i), q_max(i));
                return;
            }
        }

        KDL::ChainIkSolverPos_LMA ik_solver(kdl_chain_, 1e-4, 500, 1e-6); 
        KDL::JntArray q_init(kdl_chain_.getNrOfJoints()); 
        KDL::JntArray q_result(kdl_chain_.getNrOfJoints()); 
        int result = ik_solver.CartToJnt(q_init, target_pose, q_result);

        // // Newton-Raphson có giới hạn
        // KDL::ChainIkSolverPos_NR_JL ik_solver(kdl_chain_, q_min, q_max, fk_solver, ik_vel_solver, 200, 1e-6);
        // KDL::JntArray q_init(kdl_chain_.getNrOfJoints());
        // KDL::JntArray q_result(kdl_chain_.getNrOfJoints());
        // int result = ik_solver.CartToJnt(q_init, target_pose, q_result);

        // 3. Prepare Response
        rviz_custom_plugins::msg::KinematicsIKResponse res;
        res.joint_names = joint_names_;

        if (result >= 0) {
            res.status = "ok";
            RCLCPP_INFO(this->get_logger(), "✅ IK solved successfully.");

            // ===== 🎯 ÁP DỤNG GIỚI HẠN GÓC =====
            for (size_t i = 0; i < joint_names_.size(); i++) {
                auto name = joint_names_[i];
                double q = q_result(i);

                double lower = joint_limits_[name].first;
                double upper = joint_limits_[name].second;

                if (q < lower) {
                    RCLCPP_WARN(this->get_logger(), "⚠️ %s thấp hơn limit (%.3f -> %.3f)", name.c_str(), q, lower);
                    q = lower;
                }
                if (q > upper) {
                    RCLCPP_WARN(this->get_logger(), "⚠️ %s vượt limit (%.3f -> %.3f)", name.c_str(), q, upper);
                    q = upper;
                }

                q_result(i) = q;
            }
                
            // In từng joint ra màn hình
            std::stringstream ss;
            ss << "🔧 Joint result:";
            for (unsigned int i = 0; i < joint_names_.size(); i++) {
                double rad = q_result(i);
                double deg = rad * 180.0 / M_PI;
                ss << "\n  - " << joint_names_[i] 
                   << ": " << std::fixed << std::setprecision(4) << rad 
                   << " rad (" << std::setprecision(2) << deg << "°)";
            }
            RCLCPP_INFO(this->get_logger(), "%s", ss.str().c_str());
        
            // Lưu vào response
            for (unsigned int i = 0; i < joint_names_.size(); i++)
                res.joint_positions.push_back(q_result(i));
        
            // ===== PUBLISH JOINT STATE TO RVIZ =====
            sensor_msgs::msg::JointState js;
            js.header.stamp = this->now();
            js.name = joint_names_;
            for (unsigned int i = 0; i < joint_names_.size(); i++)
                js.position.push_back(q_result(i));
        
            joint_state_pub_->publish(js);
        
        } else {
            res.status = "fail";
            RCLCPP_ERROR(this->get_logger(), "❌ IK solver failed!");
        }
        


        // // 4. Publish response back to panel
        ik_res_pub_->publish(res);

    }

    // void handleIKRequest(const rviz_custom_plugins::msg::KinematicsIKRequest::SharedPtr msg)
    // {
    //     if (joint_names_.size() != static_cast<size_t>(kdl_chain_.getNrOfJoints())) {
    //         RCLCPP_ERROR(this->get_logger(), "❌ joint_names_ size mismatch! (%zu vs %d)",
    //             joint_names_.size(), kdl_chain_.getNrOfJoints());
    //         return;
    //     }
    //     RCLCPP_INFO(this->get_logger(), "📨 Nhận yêu cầu IK");

    //     // Build KDL target pose from msg
    //     KDL::Frame target_pose(
    //         KDL::Rotation::Quaternion(msg->target_pose.orientation.x,
    //                                   msg->target_pose.orientation.y,
    //                                   msg->target_pose.orientation.z,
    //                                   msg->target_pose.orientation.w),
    //         KDL::Vector(msg->target_pose.position.x,
    //                     msg->target_pose.position.y,
    //                     msg->target_pose.position.z));

    //     double tx = msg->target_pose.position.x;
    //     double ty = msg->target_pose.position.y;
    //     double tz = msg->target_pose.position.z;
    //     double r, p, y;
    //     target_pose.M.GetRPY(r, p, y);
    //     RCLCPP_INFO(this->get_logger(), "Target pose: pos=[%.4f, %.4f, %.4f], rpy=[%.3f, %.3f, %.3f]", tx, ty, tz, r, p, y);

    //     // Prepare FK/IK velocity solvers
    //     KDL::ChainFkSolverPos_recursive fk_solver(kdl_chain_);
    //     KDL::ChainIkSolverVel_pinv ik_vel_solver(kdl_chain_);

    //     // Build q_min / q_max and validate joint_limits_ contains every joint
    //     KDL::JntArray q_min(kdl_chain_.getNrOfJoints()), q_max(kdl_chain_.getNrOfJoints());
    //     for (size_t i = 0; i < joint_names_.size(); ++i) {
    //         const auto &name = joint_names_[i];
    //         auto it = joint_limits_.find(name);
    //         if (it == joint_limits_.end()) {
    //             RCLCPP_ERROR(this->get_logger(), "❌ joint_limits_ missing for joint '%s' — add it to the map!", name.c_str());
    //             return;
    //         }
    //         q_min(i) = it->second.first;
    //         q_max(i) = it->second.second;
    //         // Log limits
    //         RCLCPP_INFO(this->get_logger(), "Joint %s limits: [%.4f, %.4f]", name.c_str(), q_min(i), q_max(i));
    //         if (q_min(i) > q_max(i)) {
    //             RCLCPP_ERROR(this->get_logger(), "❌ For joint %s, min > max (%.4f > %.4f). Fix limits.", name.c_str(), q_min(i), q_max(i));
    //             return;
    //         }
    //     }

    //     // Prepare initial guess q_init.
    //     KDL::JntArray q_init(kdl_chain_.getNrOfJoints());
    //     for (unsigned int i = 0; i < kdl_chain_.getNrOfJoints(); ++i) {
    //         q_init(i) = current_joint_positions_[i];
    //     }

    //     // Log q_init
    //     std::stringstream qinit_ss;
    //     qinit_ss << "q_init: ";
    //     for (unsigned int i = 0; i < static_cast<unsigned int>(kdl_chain_.getNrOfJoints()); ++i)
    //         qinit_ss << q_init(i) << (i+1 < static_cast<unsigned int>(kdl_chain_.getNrOfJoints()) ? ", " : "");
    //     RCLCPP_INFO(this->get_logger(), "%s", qinit_ss.str().c_str());

    //     // Try Newton-Raphson with Joint Limits (NR_JL)
    //     KDL::ChainIkSolverPos_NR_JL ik_nrjl(kdl_chain_, q_min, q_max, fk_solver, ik_vel_solver, 1000, 1e-3);
    //     KDL::JntArray q_result(kdl_chain_.getNrOfJoints());
    //     int result = ik_nrjl.CartToJnt(q_init, target_pose, q_result);

    //     if (result >= 0) {
    //         RCLCPP_INFO(this->get_logger(), "✅ IK (NR_JL) solved successfully.");
    //     } else {
    //         RCLCPP_WARN(this->get_logger(), "⚠️ IK (NR_JL) failed (code=%d). Trying fallback: ChainIkSolverPos_LMA", result);
    //         // Fallback: try LMA (may return values outside limits — we'll clamp after)
    //         KDL::ChainIkSolverPos_LMA ik_lma(kdl_chain_, 1e-3, 1000, 0.01);

    //         result = ik_lma.CartToJnt(q_init, target_pose, q_result);
    //         if (result >= 0) {
    //             RCLCPP_INFO(this->get_logger(), "✅ IK (LMA) succeeded as fallback (will clamp to limits).");
    //             // clamp to limits (safety)
    //             for (unsigned int i = 0; i < q_result.rows(); ++i) {
    //                 if (q_result(i) < q_min(i)) {
    //                     RCLCPP_WARN(this->get_logger(), "Clamping joint %s: %.4f -> %.4f", joint_names_[i].c_str(), q_result(i), q_min(i));
    //                     q_result(i) = q_min(i);
    //                 }
    //                 if (q_result(i) > q_max(i)) {
    //                     RCLCPP_WARN(this->get_logger(), "Clamping joint %s: %.4f -> %.4f", joint_names_[i].c_str(), q_result(i), q_max(i));
    //                     q_result(i) = q_max(i);
    //                 }
    //             }
    //         } else {
    //             RCLCPP_ERROR(this->get_logger(), "❌ Both NR_JL and LMA failed to solve IK. Aborting.");
    //             // Prepare and publish fail response
    //             rviz_custom_plugins::msg::KinematicsIKResponse res;
    //             res.joint_names = joint_names_;
    //             res.status = "fail";
    //             ik_res_pub_->publish(res);
    //             return;
    //         }
    //     }

    //     // If we reach here, q_result contains a candidate solution (within limits if NR_JL or clamped after LMA)
    //     // Log q_result
    //     {
    //         std::stringstream ss;
    //         ss << "🔧 Joint result:";
    //         for (unsigned int i = 0; i < q_result.rows(); ++i) {
    //             double rad = q_result(i);
    //             double deg = rad * 180.0 / M_PI;
    //             ss << "\n  - " << joint_names_[i] << ": " << std::fixed << std::setprecision(4) << rad << " rad (" << std::setprecision(2) << deg << "°)";
    //         }
    //         RCLCPP_INFO(this->get_logger(), "%s", ss.str().c_str());
    //     }

    //     // Verify achieved pose via FK (optional but useful)
    //     KDL::Frame achieved;
    //     fk_solver.JntToCart(q_result, achieved);
    //     double ax, ay, az, ar, ap, ayaw;
    //     ax = achieved.p.x(); ay = achieved.p.y(); az = achieved.p.z();
    //     achieved.M.GetRPY(ar, ap, ayaw);
    //     RCLCPP_INFO(this->get_logger(), "Achieved pose via FK: pos=[%.4f, %.4f, %.4f], rpy=[%.3f, %.3f, %.3f]", ax, ay, az, ar, ap, ayaw);

    //     // Prepare response & publish
    //     rviz_custom_plugins::msg::KinematicsIKResponse res;
    //     res.joint_names = joint_names_;
    //     res.status = "ok";
    //     for (unsigned int i = 0; i < q_result.rows(); ++i)
    //         res.joint_positions.push_back(q_result(i));

    //     // Publish joint state for visualization
    //     sensor_msgs::msg::JointState js;
    //     js.header.stamp = this->now();
    //     js.name = joint_names_;
    //     for (unsigned int i = 0; i < q_result.rows(); ++i)
    //         js.position.push_back(q_result(i));
    //     joint_state_pub_->publish(js);

    //     ik_res_pub_->publish(res);
    // }



    // Các biến thành viên
    KDL::Tree kdl_tree_;
    KDL::Chain kdl_chain_;
    rclcpp::Subscription<rviz_custom_plugins::msg::KinematicsFKRequest>::SharedPtr subscription_;
    rclcpp::Publisher<rviz_custom_plugins::msg::KinematicsFKResponse>::SharedPtr fk_publisher_;
    rclcpp::Subscription<rviz_custom_plugins::msg::KinematicsIKRequest>::SharedPtr ik_req_sub_;
    rclcpp::Publisher<rviz_custom_plugins::msg::KinematicsIKResponse>::SharedPtr ik_res_pub_;
    std::vector<std::string> joint_names_;
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_state_pub_;
    std::vector<rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr> pos_subs_;




};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<KinematicsNode>());
    rclcpp::shutdown();
    return 0;
}

