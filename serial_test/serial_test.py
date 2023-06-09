import sys
import threading
import time
import serial
import serial.tools.list_ports
from ui_serial_test import Ui_MainWindow
from PyQt5.QtWidgets import QApplication, QMainWindow, QAction, QActionGroup, QMessageBox, QFileDialog
from PyQt5.QtGui import QKeyEvent
from PyQt5.QtCore import Qt, QFile, QTimer, QSignalMapper, pyqtSlot

def isint(value):
    try:
        int(value)
        return True
    except ValueError:
        return False

class MainWindow(QMainWindow):
    ui = Ui_MainWindow()
    ser = serial.Serial()
    tim = QTimer()
    timPeriodMs = 100
    req = QTimer()
    reqPeriodMs = 500
    triggerActivateStatus = 0
    triggerFireStatus = 0
    triggerSingleStatus = 0
    
    def __init__(self):
        super(MainWindow, self).__init__()
        self.ui.setupUi(self)
        group = QActionGroup(self.ui.menuPort)
        group.setExclusive(True)
        self.signalMapper = QSignalMapper(self)
        self.signalMapper.mapped[str].connect(self.setPort)
        for port in list(serial.tools.list_ports.comports()):
            node = QAction(port.device, self.ui.menuPort, checkable = True)
            node.triggered.connect(self.signalMapper.map)
            self.signalMapper.setMapping(node, port.device)
            group.addAction(node)
            self.ui.menuPort.addAction(node)
        self.tim.timeout.connect(self.readPort)
        self.req.timeout.connect(self.readData)

    def keyPressEvent(self, event):
        if type(event) == QKeyEvent and not event.isAutoRepeat():
            if event.key() == Qt.Key_W:
                self.on_pushButtonUp_pressed()
            elif event.key() == Qt.Key_S:
                self.on_pushButtonDown_pressed()
            elif event.key() == Qt.Key_A:
                self.on_pushButtonLeft_pressed()
            elif event.key() == Qt.Key_D:
                self.on_pushButtonRight_pressed()
            elif event.key() == Qt.Key_Shift:
                self.ui.pushButtonTriggerActivate.setChecked(True)
                self.on_pushButtonTriggerActivate_clicked(True)
            elif event.key() == Qt.Key_Control:
                self.on_pushButtonTriggerFire_pressed()
            event.accept()
        else:
            event.ignore()

    def keyReleaseEvent(self, event):
        if type(event) == QKeyEvent and not event.isAutoRepeat():
            if event.key() == Qt.Key_W:
                self.on_pushButtonUp_released()
            elif event.key() == Qt.Key_S:
                self.on_pushButtonDown_released()
            elif event.key() == Qt.Key_A:
                self.on_pushButtonLeft_released()
            elif event.key() == Qt.Key_D:
                self.on_pushButtonRight_released()
            elif event.key() == Qt.Key_Shift:
                self.ui.pushButtonTriggerActivate.setChecked(False)
                self.on_pushButtonTriggerActivate_clicked(False)
            elif event.key() == Qt.Key_Control:
                self.on_pushButtonTriggerFire_released()
            event.accept()
        else:
            event.ignore()

    def closeEvent(self, event):
        choice = QMessageBox.question(self, 'Exit', 'Are you sure?', QMessageBox.Yes| QMessageBox.No)
        if choice == QMessageBox.Yes:
            if self.ser.isOpen():
                self.ser.close()
            event.accept()
        else:
            event.ignore()

    def sendPacket(self, data):
        if self.ser.isOpen():
            packet = bytearray()
            crc = 0
            packet.append(0x21)
            for i in range (0, 7):
                if (len(data) > i):
                    packet.append(data[i])
                else:
                    packet.append(0x00)
            for one in packet:
                crc = crc + one
            packet.append((crc >> 8) & 0xFF)
            packet.append(crc & 0xFF)
            #print ' '.join('0x{:02X}'.format(x) for x in packet)
            self.ser.write(packet)
        else:
            self.ui.statusbar.showMessage('Port not open')

    def readData(self):
        packet = bytearray()
        packet.append(0x00)
        self.sendPacket(packet)
            
    def on_pushButtonMove_released(self):
        packet = bytearray()
        packet.append(0x01)
        if isint(self.ui.lineEditRotAngle.text()):
            rotAngle = int(self.ui.lineEditRotAngle.text())
            packet.append(((abs(rotAngle) >> 8) & 0xFF) | ((1 << 7) if (rotAngle < 0) else 0))
            packet.append(abs(rotAngle) & 0xFF)
        else:
            packet.append(0x00)
            packet.append(0x00)
        if isint(self.ui.lineEditTiltAngle.text()):
            tiltAngle = int(self.ui.lineEditTiltAngle.text())
            packet.append(((abs(tiltAngle) >> 8) & 0xFF) | ((1 << 7) if (tiltAngle < 0) else 0))
            packet.append(abs(tiltAngle) & 0xFF)
        else:
            packet.append(0x00)
            packet.append(0x00)
        if isint(self.ui.lineEditRotSpeed.text()):
            packet.append(abs(int(self.ui.lineEditRotSpeed.text())) & 0xFF)
        else:
            packet.append(0x00)
        if isint(self.ui.lineEditTiltSpeed.text()):    
            packet.append(abs(int(self.ui.lineEditTiltSpeed.text())) & 0xFF)
        else:
            packet.append(0x00)
        self.sendPacket(packet)
        self.ui.lineEditRotAngle.clear()
        self.ui.lineEditTiltAngle.clear()
        self.ui.lineEditRotSpeed.clear()
        self.ui.lineEditTiltSpeed.clear()

    def on_pushButtonZero_released(self):
        packet = bytearray()
        packet.append(0x04)
        self.sendPacket(packet)

    def on_pushButtonLeft_pressed(self):
        packet = bytearray()
        packet.append(0x03)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        if isint(self.ui.lineEditRotSpeed.text()):
            speed = abs(int(self.ui.lineEditRotSpeed.text()))
            if speed > 0x7F:
                packet.append(0xFF)
            else:
                packet.append((speed & 0x7F) | 0x80)
        else:
            self.ui.lineEditRotSpeed.setText(str(-1))
            packet.append(0x81)
        self.sendPacket(packet)
        
    def on_pushButtonLeft_released(self):
        packet = bytearray()
        packet.append(0x02)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        self.sendPacket(packet)
        self.ui.lineEditRotSpeed.clear()
        
    def on_pushButtonRight_pressed(self):
        packet = bytearray()
        packet.append(0x03)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        if isint(self.ui.lineEditRotSpeed.text()):
            speed = abs(int(self.ui.lineEditRotSpeed.text()))
            if speed > 0x7F:
                packet.append(0x7F)
            else:
                packet.append(speed & 0x7F)
        else:
            self.ui.lineEditRotSpeed.setText(str(1))
            packet.append(0x01)
        self.sendPacket(packet)
            
    def on_pushButtonRight_released(self):
        packet = bytearray()
        packet.append(0x02)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        self.sendPacket(packet)
        self.ui.lineEditRotSpeed.clear()

    def on_pushButtonDown_pressed(self):
        packet = bytearray()
        packet.append(0x03)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x01)
        if isint(self.ui.lineEditTiltSpeed.text()):
            speed = abs(int(self.ui.lineEditTiltSpeed.text()))
            if speed > 0x7F:
                packet.append(0xFF)
            else:    
                packet.append((speed & 0x7F) | 0x80)
        else:
            self.ui.lineEditTiltSpeed.setText(str(-1))
            packet.append(0x81)
        self.sendPacket(packet)

    def on_pushButtonDown_released(self):
        packet = bytearray()
        packet.append(0x02)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x01)
        self.sendPacket(packet)
        self.ui.lineEditTiltSpeed.clear()

    def on_pushButtonUp_pressed(self):
        packet = bytearray()
        packet.append(0x03)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x01)
        if isint(self.ui.lineEditTiltSpeed.text()):
            speed = abs(int(self.ui.lineEditTiltSpeed.text()))
            if speed > 0x7F:
                packet.append(0x7F)
            else:
                packet.append(speed & 0x7F)
        else:
            self.ui.lineEditTiltSpeed.setText(str(1))
            packet.append(0x01)
        self.sendPacket(packet)

    def on_pushButtonUp_released(self):
        packet = bytearray()
        packet.append(0x02)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x01)
        self.sendPacket(packet)
        self.ui.lineEditTiltSpeed.clear()

    def on_pushButtonStop_released(self):
        packet = bytearray()
        packet.append(0x02)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        self.sendPacket(packet)
        self.ui.lineEditRotSpeed.clear()
        packet = bytearray()
        packet.append(0x02)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x01)
        self.sendPacket(packet)
        self.ui.lineEditTiltSpeed.clear()

    def sendFirePacket(self):
        packet = bytearray()
        packet.append(0x05)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append((self.triggerActivateStatus << 0) | (self.triggerFireStatus << 1) | (self.triggerSingleStatus << 2))
        self.sendPacket(packet)

    @pyqtSlot(bool)
    def on_pushButtonTriggerActivate_clicked(self, arg):
        if arg:
            self.triggerActivateStatus = 1
        else:
            self.triggerActivateStatus = 0
            self.triggerFireStatus = 0
        self.sendFirePacket()

    def on_pushButtonTriggerFire_pressed(self):
        if self.triggerActivateStatus == 1:
            self.triggerFireStatus = 1
            self.sendFirePacket()
        else:
            self.triggerSingleStatus = 1
            self.sendFirePacket()
            self.triggerSingleStatus = 0

    def on_pushButtonTriggerFire_released(self):
        if self.triggerActivateStatus == 1:
            self.triggerFireStatus = 0
            self.sendFirePacket()

    @pyqtSlot(bool)
    def on_actionOpen_triggered(self, arg):
        name, _ = QFileDialog.getOpenFileName(self)
        if QFile.exists(name):
            with open(name, 'r') as file:
                text = file.read()
                self.ui.textEdit.setText(text)
                file.close()

    @pyqtSlot(bool)
    def on_actionSave_triggered(self, arg):
        name, _ = QFileDialog.getSaveFileName(self)
        if QFile.exists(name):
            with open(name, 'w') as file:
                text = self.ui.textEdit.toPlainText()
                file.write(text)
                file.close()

    @pyqtSlot(bool)
    def on_actionExit_triggered(self, arg):
        choice = QMessageBox.question(self, 'Exit', 'Are you sure?', QMessageBox.Yes | QMessageBox.No)
        if choice == QMessageBox.Yes:
            sys.exit()

    def setPort(self, name):
        self.ser = serial.Serial()
        self.ser.port = str(name)
        self.ser.baudrate = 9600
        self.ser.parity = serial.PARITY_NONE
        self.ser.stopbits = serial.STOPBITS_ONE
        self.ser.bytesize = serial.EIGHTBITS
        try: 
            self.ser.open()
        except Exception:
            self.ui.statusbar.showMessage('Port ' + name + ' not available')
        if self.ser.isOpen():
            self.tim.start(self.timPeriodMs)
            self.req.start(self.reqPeriodMs)
            self.ui.statusbar.showMessage('Connected to ' + name)

    def readPort(self):
        crc = 0
        if self.ser.isOpen() and (self.ser.inWaiting() >= 10):
            data = map(ord, self.ser.read(10))
            for i in range (0, len(data) - 2):
                crc = crc + data[i]
            if crc == ((data[8] << 8) | data[9]):
                #print ' '.join('0x{:02X}'.format(x) for x in data)
                if (data[0] == 0xA1):
                    if data[1] == 0xFF:
                        self.ui.statusbar.showMessage('Acknowledged')
                    elif data[1] == 0xFE:
                        rotAngle = int(((data[2] & 0x7F) << 8) | data[3])
                        if (data[2] & 0x80) != 0:
                            rotAngle = -rotAngle
                        tiltAngle = int(((data[4] & 0x7F) << 8) | data[5])
                        if (data[4] & 0x80) != 0:
                            tiltAngle = -tiltAngle
                        self.ui.lineEditRealRot.setText(str(rotAngle))
                        self.ui.lineEditRealTilt.setText(str(tiltAngle))
                        messageText = ''
                        if (data[6] & (1 << 6)):
                            messageText = messageText + 'Using gyroscope '
                        if (data[6] & (1 << 7)):
                            messageText = messageText + 'Gyroscope error '
                        if (data[6] & (1 << 0)):
                            messageText = messageText + 'Rotate finsih '
                        if (data[6] & (1 << 2)):
                            messageText = messageText + 'Rotate active '
                        if (data[6] & (1 << 4)):
                            messageText = messageText + 'Rotate error '
                        if (data[6] & (1 << 1)):
                            messageText = messageText + 'Tilt finsih '
                        if (data[6] & (1 << 3)):
                            messageText = messageText + 'Tilt active '
                        if (data[6] & (1 << 5)):
                            messageText = messageText + 'Tilt error '
                        if (data[7] & (1 << 0)):
                            messageText = messageText + 'Calibrate in progess '
                        if (data[7] & (1 << 1)):
                            messageText = messageText + 'Need calibrate '
                        self.ui.statusbar.showMessage(messageText)
                    else:
                        self.ui.statusbar.showMessage('Unknown')

app = QApplication(sys.argv)
main = MainWindow()
main.show()
sys.exit(app.exec_())
