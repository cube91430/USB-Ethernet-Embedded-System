import sys
import serial
import serial.tools.list_ports
import time
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QLabel,
    QPushButton, QComboBox, QTextEdit, QTabWidget, QMessageBox, QLineEdit,
    QTableWidget, QTableWidgetItem, QGroupBox, QGridLayout, QFrame
)
from PyQt5.QtCore import QTimer, Qt

class MicrocontrollerPanel(QWidget):
    def __init__(self, name):
        self.bound_device_id = None
        super().__init__()
        self.name = name
        self.serial_port = None
        self.timer = QTimer()
        self.timer.timeout.connect(self.read_serial_data)
        self.last_time = time.time()
        self.init_ui()

    def init_ui(self):
        main_layout = QHBoxLayout()
        layout = QVBoxLayout()

        top_bar = QHBoxLayout()
        self.com_combo = QComboBox()
        self.refresh_ports()

        self.baud_combo = QComboBox()
        self.baud_combo.addItems(["9600", "19200", "38400", "57600", "115200"])

        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self.toggle_connection)

        top_bar.addWidget(QLabel("COM Port:"))
        top_bar.addWidget(self.com_combo)
        top_bar.addWidget(QLabel("Baudrate:"))
        top_bar.addWidget(self.baud_combo)
        top_bar.addWidget(self.connect_button)

        self.status_bar = QHBoxLayout()
        self.time_diff_label = QLabel("Δt: 0 ms")
        self.device_id_input = QLineEdit()
        self.device_id_input.setPlaceholderText("Device ID")
        self.device_id_input.setFixedWidth(100)
        self.status_label = QLabel("Status: Idle")
        self.status_bar.addWidget(self.time_diff_label)
        self.status_bar.addWidget(QLabel("Device ID:"))
        self.status_bar.addWidget(self.device_id_input)
        self.status_bar.addStretch()
        self.status_bar.addWidget(self.status_label)

        self.output_display = QTextEdit()
        self.output_display.setReadOnly(True)
        self.output_display.setPlaceholderText(f"Serial output for {self.name} will appear here...")

        send_bar = QHBoxLayout()
        self.input_line = QLineEdit()
        self.input_line.setPlaceholderText("Type command to send...")
        self.send_button = QPushButton("Send")
        self.send_button.clicked.connect(self.send_serial_data)

        send_bar.addWidget(self.input_line)
        send_bar.addWidget(self.send_button)

        table_bar = QHBoxLayout()

        self.sensor_table = QTableWidget(1, 4)
        self.sensor_table.setHorizontalHeaderLabels(["Temperature", "Humidity", "Smoke Detection", "Flame Detection"])
        for col in range(4):
            self.sensor_table.setItem(0, col, QTableWidgetItem("-"))

        self.electrical_table = QTableWidget(1, 5)
        self.electrical_table.setHorizontalHeaderLabels(["Voltage", "Current", "Power", "Apparent Power", "kV Rating"])
        for col in range(5):
            self.electrical_table.setItem(0, col, QTableWidgetItem("-"))

        table_bar.addWidget(self.sensor_table)
        table_bar.addWidget(self.electrical_table)

        control_group = QGroupBox("Control and Trigger")
        control_layout = QGridLayout()
        buttons = ["Fan 1", "Fan 2", "Pump 1", "Pump 2", "Light 1", "Light 2", "Suppression"]

        for i, label in enumerate(buttons):
            button = QPushButton(label)
            control_layout.addWidget(button, i // 4, i % 4)

        control_group.setLayout(control_layout)

        # Configuration frame on the right
        config_frame = QVBoxLayout()
        config_group = QGroupBox("Pin Configuration")

        analog_group = QGroupBox("Analog")
        analog_layout = QVBoxLayout()
        for i in range(1, 7):
            row = QHBoxLayout()
            row.addWidget(QLabel(f"ADC{i}"))
            mode_combo = QComboBox()
            mode_combo.addItems(["Analog", "Digital"])
            value_display = QLineEdit("0")
            value_display.setReadOnly(True)
            row.addWidget(mode_combo)
            row.addWidget(value_display)
            analog_layout.addLayout(row)
        analog_group.setLayout(analog_layout)

        digital_group = QGroupBox("Digital")
        digital_layout = QVBoxLayout()
        digital_pins = ["DIGA", "DIGB", "DIGC", "DIGD", "DIGE", "DIGF", "DIGH", "DIGJ", "DIGK"]
        for pin in digital_pins:
            row = QHBoxLayout()
            row.addWidget(QLabel(pin))
            mode_combo = QComboBox()
            mode_combo.addItems(["Digital", "UART", "SPI", "I2C"])
            value_display = QLineEdit("0")
            value_display.setReadOnly(True)

            def handle_mode_change(combo=mode_combo, value_box=value_display):
                if combo.currentText() in ["UART", "SPI", "I2C"]:
                    value_box.setDisabled(True)
                else:
                    value_box.setDisabled(False)

            mode_combo.currentTextChanged.connect(lambda _, c=mode_combo, v=value_display: handle_mode_change(c, v))

            row.addWidget(mode_combo)
            row.addWidget(value_display)
            digital_layout.addLayout(row)
        digital_group.setLayout(digital_layout)

        signal_group = QGroupBox("Signal")
        signal_layout = QVBoxLayout()
        self.interval_input = QLineEdit()
        self.interval_input.setPlaceholderText("Interval in microseconds")
        signal_layout.addWidget(QLabel("Send Interval (μs):"))
        signal_layout.addWidget(self.interval_input)
        write_button = QPushButton("Write")
        signal_layout.addWidget(write_button)
        signal_group.setLayout(signal_layout)

        config_layout = QVBoxLayout()
        config_layout.addWidget(analog_group)
        config_layout.addWidget(digital_group)
        config_layout.addWidget(signal_group)
        config_group.setLayout(config_layout)

        config_frame.addWidget(config_group)

        layout.addLayout(top_bar)
        layout.addLayout(self.status_bar)
        layout.addWidget(self.output_display)
        layout.addLayout(send_bar)
        layout.addLayout(table_bar)
        layout.addWidget(control_group)

        main_layout.addLayout(layout, 3)
        main_layout.addLayout(config_frame, 1)
        self.setLayout(main_layout)

    def refresh_ports(self):
        ports = serial.tools.list_ports.comports()
        self.com_combo.clear()
        for port in ports:
            self.com_combo.addItem(port.device)

    def toggle_connection(self):
        if self.serial_port and self.serial_port.is_open:
            self.disconnect_serial()
        else:
            self.connect_serial()

    def connect_serial(self):
        port = self.com_combo.currentText()
        try:
            baud = int(self.baud_combo.currentText())
            self.serial_port = serial.Serial(port, baud, timeout=0.1)
            self.connect_button.setText("Disconnect")
            self.status_label.setText("Status: On")
            self.timer.start(100)
        except Exception as e:
            QMessageBox.critical(self, f"Connection Failed - {self.name}", f"Could not open port:\n{e}")

    def disconnect_serial(self):
        if self.serial_port and self.serial_port.is_open:
            self.timer.stop()
            self.serial_port.close()
            self.connect_button.setText("Connect")
            self.status_label.setText("Status: Off")

    def read_serial_data(self):
        if self.serial_port and self.serial_port.in_waiting > 0:
            try:
                current_time = time.time()
                delta_ms = int((current_time - self.last_time) * 1000)
                self.last_time = current_time
                self.time_diff_label.setText(f"Δt: {delta_ms} ms")
                self.status_label.setText("Status: Active")

                line = self.serial_port.readline().decode("utf-8", errors="ignore").strip()
                if line:
                    self.output_display.append(line)
                    parts = line.split('#')
                    if len(parts) >= 7:
                        device_id, data_id, dlc = parts[0:3]
                        if not self.bound_device_id:
                            self.bound_device_id = device_id
                            self.device_id_input.setText(device_id)

                        if device_id == self.bound_device_id:
                            # Example of filtering by specific data_id
                            if data_id == "0345":
                                temperature = parts[3]
                                humidity = parts[4]
                                smoke = parts[5]
                                flame = parts[6]

                                self.sensor_table.setItem(0, 0, QTableWidgetItem(temperature))
                                self.sensor_table.setItem(0, 1, QTableWidgetItem(humidity))
                                self.sensor_table.setItem(0, 2, QTableWidgetItem(smoke))
                                self.sensor_table.setItem(0, 3, QTableWidgetItem(flame))
                            if data_id == "4567":
                                temperature = parts[3]
                                humidity = parts[4]
                                smoke = parts[5]
                                flame = parts[6]

                                self.sensor_table.setItem(0, 0, QTableWidgetItem(temperature))
                                self.sensor_table.setItem(0, 1, QTableWidgetItem(humidity))
                                self.sensor_table.setItem(0, 2, QTableWidgetItem(smoke))
                                self.sensor_table.setItem(0, 3, QTableWidgetItem(flame))
            except Exception as e:
                self.output_display.append(f"[Error reading data] {e}")
        else:
            self.status_label.setText("Status: Idle")

    def send_serial_data(self):
        if self.serial_port and self.serial_port.is_open:
            message = self.input_line.text().strip()
            if message:
                try:
                    self.serial_port.write((message + "\n").encode())
                    self.output_display.append(f"> {message}")
                    self.input_line.clear()
                except Exception as e:
                    QMessageBox.critical(self, f"Send Failed - {self.name}", f"Could not send message:\n{e}")
        else:
            QMessageBox.warning(self, f"Not Connected - {self.name}", "Please connect to the serial port first.")

class MultiMCUGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Multi-MCU Serial Monitor")
        self.setGeometry(100, 100, 1200, 700)

        tab_widget = QTabWidget()
        tab_widget.addTab(MicrocontrollerPanel("MCU 1"), "MCU 1")
        tab_widget.addTab(MicrocontrollerPanel("MCU 2"), "MCU 2")
        tab_widget.addTab(MicrocontrollerPanel("MCU 3"), "MCU 3")
        tab_widget.addTab(MicrocontrollerPanel("MCU 4"), "MCU 4")

        self.setCentralWidget(tab_widget)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MultiMCUGUI()
    window.show()
    sys.exit(app.exec_())
