#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <Eigen/Dense>

// Pinocchio
#include <pinocchio/multibody/model.hpp>
#include <pinocchio/multibody/data.hpp>
#include <pinocchio/parsers/urdf.hpp>
#include <pinocchio/algorithm/rnea.hpp>

// ODrive interface (bạn đã có package này)
#include <can_odrive_interface/odrive_interface.hpp>

/// Cấu trúc tham số để hiệu chỉnh gravity compensation
struct GravityCompParams
{
    std::vector<double> mass_scaling;   // scale mô-men theo từng khớp
    std::vector<double> friction_comp;  // bù ma sát (Nm)
    std::vector<double> torque_limits;  // giới hạn |tau| tối đa (Nm)
    Eigen::Vector3d     gravity_vector; // vector trọng lực (m/s^2)

    explicit GravityCompParams(int num_joints);
};

/// Lớp tính toán gravity compensation cho robot 3 khớp (base, shoulder, elbow)
class ArmGravityComp
{
public:
    /// @param urdf_path: đường dẫn đến file URDF (rehab_robot_ver2.urdf)
    explicit ArmGravityComp(const std::string& urdf_path);
    ~ArmGravityComp();

    /// Cập nhật góc khớp từ vector rad (nếu bạn muốn dùng nguồn khác ODrive)
    // void UpdatePositions(const std::vector<double>& positions);

    /// Cập nhật góc khớp trực tiếp từ ODrive.
    /// Giả định: 
    ///  - node/axis 0 -> base_joint
    ///  - node/axis 1 -> shoulder_joint
    ///  - node/axis 2 -> elbow_joint
    ///  - odrive.get_encoder_data(id) trả về góc theo rad (nếu không, convert trong .cpp)
    void UpdateFromOdrive(can_odrive_interface::ODriveInterface& odrive);

    /// Tính torque bù trọng lực (Nm) cho trạng thái q_ hiện tại.
    void ComputeGravityCompensation();

    /// Lấy torque bù trọng lực của 1 khớp (Nm)
    double GetGravityTorque(int joint) const;

    /// Lấy vector torque bù trọng lực của tất cả khớp (Nm)
    std::vector<double> GetGravityTorques() const;

    /// Set tham số hiệu chỉnh (scale khối lượng, ma sát, giới hạn mô-men, vector trọng lực)
    void SetCalibrationParams(const GravityCompParams& params);

    /// Lấy tên các joint có DOF (để map với motor)
    std::vector<std::string> GetJointNames() const;

    /// In trạng thái hiện tại (debug)
    void PrintStatus() const;

private:
    pinocchio::Model model_;
    pinocchio::Data  data_;

    Eigen::VectorXd q_;            // vị trí khớp (rad), kích thước = nv
    Eigen::VectorXd v_;            // vận tốc khớp (rad/s), hiện = 0
    Eigen::VectorXd a_;            // gia tốc khớp (rad/s^2), hiện = 0
    Eigen::VectorXd tau_gravity_;  // mô-men bù trọng lực (Nm), kích thước = nv

    GravityCompParams params_;
};
