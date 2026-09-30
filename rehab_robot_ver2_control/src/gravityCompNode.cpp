#include "rehab_robot_ver2_control/gravityCompNode.hpp"

// ================== GravityCompParams ==================

GravityCompParams::GravityCompParams(int num_joints)
    : mass_scaling(num_joints, 1.0),
      friction_comp(num_joints, 0.0),
      torque_limits(num_joints, 10.0),
      gravity_vector(0.0, 0.0, -9.81)
{
    // Bạn có thể chỉnh mặc định ở đây (vd. torque_limits theo từng khớp)
}

// ================== ArmGravityComp ==================

ArmGravityComp::ArmGravityComp(const std::string& urdf_path)
    : params_(0)
{
    std::cout << "=== Loading URDF Model ===" << std::endl;

    try {
        pinocchio::urdf::buildModel(urdf_path, model_);
        std::cout << "Full model loaded from: " << urdf_path << std::endl;
        std::cout << "  nq = " << model_.nq
                  << ", nv = " << model_.nv
                  << ", njoints = " << model_.njoints << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to load URDF: ") + e.what());
    }

    data_ = pinocchio::Data(model_);

    q_.setZero(model_.nv);
    v_.setZero(model_.nv);
    a_.setZero(model_.nv);
    tau_gravity_.setZero(model_.nv);

    params_ = GravityCompParams(model_.nv);

    std::cout << "=== Gravity Compensation Initialized ===\n"
              << "Active DOF (nv): " << model_.nv << std::endl;

    std::cout << "Active joints (with DOF > 0):\n";
    for (pinocchio::JointIndex jid = 1; jid < model_.njoints; ++jid) {
        if (model_.joints[jid].nv() > 0) {
            std::cout << "  joint id " << jid
                      << " : " << model_.names[jid]
                      << " (nv=" << model_.joints[jid].nv() << ")\n";
        }
    }

    if (model_.nv != 3) {
        std::cerr << "[ArmGravityComp] Warning: model_.nv != 3 (nv = "
                  << model_.nv
                  << "). URDF không phải đúng 3 DOF như mong đợi.\n";
    }
}

ArmGravityComp::~ArmGravityComp() = default;

// // ------------------------------------------------------
// // Cập nhật q từ vector rad (size = nv)
// // ------------------------------------------------------
// void ArmGravityComp::UpdatePositions(const std::vector<double>& positions)
// {
//     if (positions.size() != static_cast<size_t>(model_.nv)) {
//         std::cerr << "[ArmGravityComp::UpdatePositions] Warning: Expected "
//                   << model_.nv << " positions, received "
//                   << positions.size() << std::endl;
//         return;
//     }

//     Eigen::Map<const Eigen::VectorXd> q_map(positions.data(), positions.size());
//     q_ = q_map;
// }

// ------------------------------------------------------
// Cập nhật q trực tiếp từ ODrive
// Giả định:
//  - axis/node 0 -> base_joint
//  - axis/node 1 -> shoulder_joint
//  - axis/node 2 -> elbow_joint
//  - get_encoder_data(id) trả về rad; nếu không, chỉnh lại phần này
// ------------------------------------------------------
void ArmGravityComp::UpdateFromOdrive(can_odrive_interface::ODriveInterface& odrive)
{
    // Cập nhật encoder bên trong ODrive
    odrive.ReadEncoder();

    std::vector<double> positions(model_.nv, 0.0);

    // TODO: nếu get_encoder_data trả về đơn vị khác rad
    // thì convert tại đây, ví dụ:
    //  - nếu là vòng (rev): rad = rev * 2 * M_PI
    //  - nếu là tick: rad = (tick / ticks_per_rev) * 2 * M_PI

    // if (model_.nv >= 1) positions[0] = odrive.get_encoder_data(0); // base_joint
    // if (model_.nv >= 2) positions[1] = odrive.get_encoder_data(1); // shoulder_joint
    // if (model_.nv >= 3) positions[2] = odrive.get_encoder_data(2); // elbow_joint
    float pos, vel;
    odrive.get_encoder_data(2, pos, vel);
    positions[2] = pos; // elbow_joint

    std::cout << "[ArmGravityComp] Joint 2 position (rad): "
          << std::setprecision(4) << positions[2] << "\n";

    
    // UpdatePositions(positions);
}

// ------------------------------------------------------
// Tính torque bù trọng lực cho q_ hiện tại
// ------------------------------------------------------
void ArmGravityComp::ComputeGravityCompensation()
{
    // Cập nhật hướng trọng lực
    model_.gravity.linear(params_.gravity_vector);

    // v_ = 0, a_ = 0 -> RNEA chủ yếu trả về torque do gravity
    tau_gravity_ = pinocchio::rnea(model_, data_, q_, v_, a_);

    // Áp dụng mass scaling, friction offset, và giới hạn mô-men
    for (int j = 0; j < model_.nv; ++j) {
        double tau = tau_gravity_(j);

        tau = tau * params_.mass_scaling[j] + params_.friction_comp[j];

        tau = std::clamp(
            tau,
            -params_.torque_limits[j],
            params_.torque_limits[j]
        );

        tau_gravity_(j) = tau;
    }
}

// ------------------------------------------------------
// Lấy torque của 1 khớp (Nm)
// ------------------------------------------------------
double ArmGravityComp::GetGravityTorque(int joint) const
{
    if (joint < 0 || joint >= model_.nv)
        return 0.0;
    return tau_gravity_(joint);
}

// ------------------------------------------------------
// Lấy vector torque của toàn bộ khớp (Nm)
// ------------------------------------------------------
std::vector<double> ArmGravityComp::GetGravityTorques() const
{
    return std::vector<double>(tau_gravity_.data(),
                               tau_gravity_.data() + model_.nv);
}

// ------------------------------------------------------
// Set tham số hiệu chỉnh
// ------------------------------------------------------
void ArmGravityComp::SetCalibrationParams(const GravityCompParams& params)
{
    if (params.mass_scaling.size() != static_cast<size_t>(model_.nv)) {
        std::cerr << "[ArmGravityComp::SetCalibrationParams] Warning: Parameter size mismatch! Expected "
                  << model_.nv << std::endl;
        return;
    }
    params_ = params;
}

// ------------------------------------------------------
// Lấy tên các joint có DOF > 0
// ------------------------------------------------------
std::vector<std::string> ArmGravityComp::GetJointNames() const
{
    std::vector<std::string> names;
    for (pinocchio::JointIndex jid = 1; jid < model_.njoints; ++jid) {
        if (model_.joints[jid].nv() > 0) {
            names.push_back(model_.names[jid]);
        }
    }
    return names;
}

// ------------------------------------------------------
// In trạng thái hiện tại (debug)
// ------------------------------------------------------
void ArmGravityComp::PrintStatus() const
{
    std::cout << "\n=== Gravity Compensation Status ===\n"
              << "Active DOF (nv): " << model_.nv << "\nPositions (rad): [ ";

    for (int j = 0; j < model_.nv; ++j) {
        std::cout << q_(j);
        if (j < model_.nv - 1)
            std::cout << ", ";
        else
            std::cout << " ]\n";
    }

    std::cout << "Torques (Nm): [ ";
    for (int j = 0; j < model_.nv; ++j) {
        std::cout << tau_gravity_(j);
        if (j < model_.nv - 1)
            std::cout << ", ";
        else
            std::cout << " ]\n";
    }
}
