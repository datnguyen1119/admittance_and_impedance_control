# import sys
# from PyQt5 import QtWidgets, QtGui
# from rehab_gui import Ui_MainWindow   # Nhập lớp giao diện từ rehab_gui.py

# class MainWindow(QtWidgets.QMainWindow):
#     def __init__(self):
#         super().__init__()

#         self.ui = Ui_MainWindow()
#         self.ui.setupUi(self)

#         # ===== KẾT NỐI CÁC NÚT VỚI HÀM =====
#         self.ui.shoulder_extension.clicked.connect(self.show_shoulder_extension)
#         self.ui.shoulder_flexion.clicked.connect(self.show_shoulder_flexion)
#         self.ui.elbow_extension.clicked.connect(self.show_elbow_extension)
#         self.ui.elbow_flexion.clicked.connect(self.show_elbow_flexion)

#         # Optional: ảnh mặc định ban đầu
#         self.show_shoulder_extension()
#         self.ui.log_panel.appendPlainText("Robot started")


#     # ===== 4 HÀM ĐỔI ẢNH =====
#     def show_shoulder_extension(self):
#         self.update_image("../resource/shoulder_extension.png")

#     def show_shoulder_flexion(self):
#         self.update_image("../resource/shoulder_flexion.png")

#     def show_elbow_extension(self):
#         self.update_image("../resource/elbow_extension.png")

#     def show_elbow_flexion(self):
#         self.update_image("../resource/elbow_flexion.png")

#     # ===== HÀM DÙNG CHUNG ĐỂ ĐỔI ẢNH =====
#     def update_image(self, image_path):
#         pixmap = QtGui.QPixmap(image_path)

#         if pixmap.isNull():
#             print(f"[ERROR] Không tìm thấy ảnh: {image_path}")
#             return

#         self.ui.label.setPixmap(pixmap)
#         self.ui.label.setScaledContents(True)  # cho ảnh khớp label


# if __name__ == "__main__":
#     app = QtWidgets.QApplication(sys.argv)
#     window = MainWindow()
#     window.show()
#     sys.exit(app.exec_())


import sys
from PyQt5 import QtWidgets, QtGui
from rehab_gui import Ui_MainWindow
import rclpy
from rclpy.node import Node
from std_msgs.msg import String  # bạn có thể đổi sang Int32 nếu muốn

class MainWindow(QtWidgets.QMainWindow, Node):
    def __init__(self):
        QtWidgets.QMainWindow.__init__(self)
        Node.__init__(self, "rehab_gui_node")

        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        # Publisher gửi dữ liệu mode / bài tập
        self.publisher_ = self.create_publisher(String, '/exercises_mode', 10)

        # ===== KẾT NỐI CÁC NÚT VỚI HÀM =====
        self.ui.shoulder_extension.clicked.connect(lambda: self.send_mode("shoulder_extension"))
        self.ui.shoulder_flexion.clicked.connect(lambda: self.send_mode("shoulder_flexion"))
        self.ui.elbow_extension.clicked.connect(lambda: self.send_mode("elbow_extension"))
        self.ui.elbow_flexion.clicked.connect(lambda: self.send_mode("elbow_flexion"))

        # Optional: ảnh mặc định ban đầu
        self.show_shoulder_extension()
        self.ui.log_panel.appendPlainText("Robot GUI started")

    # ===== 4 HÀM ĐỔI ẢNH =====
    def show_shoulder_extension(self):
        self.update_image("../resource/shoulder_extension.png")

    def show_shoulder_flexion(self):
        self.update_image("../resource/shoulder_flexion.png")

    def show_elbow_extension(self):
        self.update_image("../resource/elbow_extension.png")

    def show_elbow_flexion(self):
        self.update_image("../resource/elbow_flexion.png")

    def update_image(self, image_path):
        pixmap = QtGui.QPixmap(image_path)
        if pixmap.isNull():
            print(f"[ERROR] Không tìm thấy ảnh: {image_path}")
            return
        self.ui.label.setPixmap(pixmap)
        self.ui.label.setScaledContents(True)

    # ===== GỬI DỮ LIỆU ROS2 =====
    def send_mode(self, mode_name):
        msg = String()
        msg.data = mode_name
        self.publisher_.publish(msg)
        self.ui.log_panel.appendPlainText(f"Sent mode: {mode_name}")


def main(args=None):
    rclpy.init(args=args)
    app = QtWidgets.QApplication(sys.argv)
    window = MainWindow()
    window.show()
    try:
        sys.exit(app.exec_())
    finally:
        rclpy.shutdown()


if __name__ == "__main__":
    main()
