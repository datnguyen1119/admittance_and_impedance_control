import os
from rqt_gui_py.plugin import Plugin
from PyQt5 import QtWidgets, QtGui
from PyQt5.QtWidgets import QWidget

import rclpy
from rclpy.node import Node
from std_msgs.msg import String

from ament_index_python.packages import get_package_share_directory
from my_rqt_plugin.rehab_gui import Ui_MainWindow


# ================= ROS2 Node riêng để publish lệnh bài tập =================
class RehabNode(Node):
    def __init__(self):
        super().__init__('rehab_rqt_node')
        self.pub_exercise = self.create_publisher(String, '/exercise_cmd', 10)
        self.get_logger().info("[RehabNode] Node started")

    def send_command(self, cmd: str):
        msg = String()
        msg.data = cmd
        self.pub_exercise.publish(msg)
        self.get_logger().info(f"[RehabNode] Published: {cmd}")


# ================= Plugin rqt =================
class RehabPlugin(Plugin):
    def __init__(self, context):
        super(RehabPlugin, self).__init__(context)
        self.setObjectName('RehabPlugin')

        # ================= ROS2 Node =================
        # Không init rclpy trực tiếp nếu rqt đã init
        if not rclpy.ok():
            rclpy.init(args=None)
        self.node = RehabNode()

        # ================= Widget từ UI =================
        self._widget = QWidget()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self._widget)

        # Thêm widget vào rqt
        if context.serial_number() > 1:
            self._widget.setWindowTitle(self._widget.windowTitle() + (' (%d)' % context.serial_number()))
        context.add_widget(self._widget)

        # ================= Lấy đường dẫn resource =================
        pkg_path = get_package_share_directory('my_rqt_plugin')
        self.resource_path = os.path.join(pkg_path, 'resource')

        # ================= Kết nối nút bấm với hàm =================
        self.ui.shoulder_extension.clicked.connect(
            lambda: self.start_exercise('shoulder_extension', 'shoulder_extension.png'))
        self.ui.shoulder_flexion.clicked.connect(
            lambda: self.start_exercise('shoulder_flexion', 'shoulder_flexion.png'))
        self.ui.elbow_extension.clicked.connect(
            lambda: self.start_exercise('elbow_extension', 'elbow_extension.png'))
        self.ui.elbow_flexion.clicked.connect(
            lambda: self.start_exercise('elbow_flexion', 'elbow_flexion.png'))

        self.ui.speedMode_slow.clicked.connect(lambda: self.set_speed('slow'))
        self.ui.speedMode_normal.clicked.connect(lambda: self.set_speed('normal'))
        self.ui.speedMode_fast.clicked.connect(lambda: self.set_speed('fast'))

        self.ui.START.clicked.connect(self.start_selected_exercise)
        self.ui.STOP.clicked.connect(self.stop_exercise)

        # Optional: log startup
        self.ui.log_panel.appendPlainText("Rehab Plugin started")

        # Lưu trạng thái bài tập hiện tại
        self.current_exercise = None
        self.current_image = None

    # ================= Các hàm tương tác =================
    def start_exercise(self, exercise_name, image_filename):
        self.current_exercise = exercise_name
        self.current_image = os.path.join(self.resource_path, image_filename)
        self.update_image(self.current_image)
        self.ui.log_panel.appendPlainText(f"Selected exercise: {exercise_name}")

    def set_speed(self, speed_mode):
        self.ui.log_panel.appendPlainText(f"Speed mode set to: {speed_mode}")

    def start_selected_exercise(self):
        if self.current_exercise:
            self.node.send_command(f"start_{self.current_exercise}")
            self.ui.log_panel.appendPlainText(f"Started: {self.current_exercise}")
        else:
            self.ui.log_panel.appendPlainText("No exercise selected!")

    def stop_exercise(self):
        self.node.send_command("stop")
        self.ui.log_panel.appendPlainText("Stopped exercise")

    # ================= Đổi ảnh =================
    def update_image(self, image_path):
        pixmap = QtGui.QPixmap(image_path)
        if pixmap.isNull():
            self.ui.log_panel.appendPlainText(f"[ERROR] Cannot load image: {image_path}")
            return
        self.ui.label.setPixmap(pixmap)
        self.ui.label.setScaledContents(True)

    # ================= Shutdown plugin =================
    def shutdown_plugin(self):
        """Called when plugin is closing"""
        if self.node is not None:
            self.node.destroy_node()
        # rclpy.shutdown()  # chỉ shutdown khi chắc chắn là plugin cuối cùng

