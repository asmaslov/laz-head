import sys
import threading
import serial, time
import serial.tools.list_ports
from ui_serial_test import Ui_MainWindow
from PyQt4 import QtCore, QtGui

def isint(value):
    try:
        int(value)
        return True
    except ValueError:
        return False

class MainWindow(QtGui.QMainWindow):
    ui = Ui_MainWindow()
    ser = serial.Serial()
    tim = QtCore.QTimer()
    
    def __init__(self):
        super(MainWindow, self).__init__()
        self.ui.setupUi(self)
        group = QtGui.QActionGroup(self.ui.menuPort, exclusive = True)
        self.signalMapper = QtCore.QSignalMapper(self)
        self.signalMapper.mapped[str].connect(self.setPort)
        for port in list(serial.tools.list_ports.comports()):
            node = QtGui.QAction(port.device, self.ui.menuPort, checkable = True)
            node.triggered.connect(self.signalMapper.map)
            self.signalMapper.setMapping(node, port.device)
            group.addAction(node)
            self.ui.menuPort.addAction(node)
        self.tim.timeout.connect(self.readPort)

    def keyPressEvent(self, event):
        if type(event) == QtGui.QKeyEvent and not event.isAutoRepeat():
            if event.key() == QtCore.Qt.Key_W:
                self.on_pushButtonUp_pressed()
            elif event.key() == QtCore.Qt.Key_S:
                self.on_pushButtonDown_pressed()
            elif event.key() == QtCore.Qt.Key_A:
                self.on_pushButtonLeft_pressed()
            elif event.key() == QtCore.Qt.Key_D:
                self.on_pushButtonRight_pressed()
            event.accept()
        else:
            event.ignore()

    def keyReleaseEvent(self, event):
        if type(event) == QtGui.QKeyEvent and not event.isAutoRepeat():
            if event.key() == QtCore.Qt.Key_W:
                self.on_pushButtonUp_released()
            elif event.key() == QtCore.Qt.Key_S:
                self.on_pushButtonDown_released()
            elif event.key() == QtCore.Qt.Key_A:
                self.on_pushButtonLeft_released()
            elif event.key() == QtCore.Qt.Key_D:
                self.on_pushButtonRight_released()
            event.accept()
        else:
            event.ignore()

    def closeEvent(self, event):
        choice = QtGui.QMessageBox.question(self, 'Exit', 'Are you sure?', QtGui.QMessageBox.Yes| QtGui.QMessageBox.No)
        if choice == QtGui.QMessageBox.Yes:
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

    def on_pushButtonRead_released(self):
        packet = bytearray()
        packet.append(0x00)
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
            packet.append(0x00)
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
            packet.append(0x00)
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

    def on_pushButtonDown_pressed(self):
        packet = bytearray()
        packet.append(0x03)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x01)
        if isint(self.ui.lineEditRotSpeed.text()):
            speed = abs(int(self.ui.lineEditTiltSpeed.text()))
            if speed > 0x7F:
                packet.append(0xFF)
            else:    
                packet.append((speed & 0x7F) | 0x80)
        else:
            packet.append(0x00)
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

    def on_pushButtonUp_pressed(self):
        packet = bytearray()
        packet.append(0x03)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x01)
        if isint(self.ui.lineEditRotSpeed.text()):
            speed = abs(int(self.ui.lineEditTiltSpeed.text()))
            if speed > 0x7F:
                packet.append(0x7F)
            else:
                packet.append(speed & 0x7F)
        else:
            packet.append(0x00)
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

    def on_pushButtonStop_released(self):
        packet = bytearray()
        packet.append(0x02)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        self.sendPacket(packet)
        packet = bytearray()
        packet.append(0x02)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x01)
        self.sendPacket(packet)
    
    @QtCore.pyqtSlot(bool)
    def on_actionOpen_triggered(self, arg):
        name = QtGui.QFileDialog.getOpenFileName(self)
        if QtCore.QFile.exists(name):
            with open(name, 'r') as file:
                text = file.read()
                self.ui.textEdit.setText(text)
                file.close()

    @QtCore.pyqtSlot(bool)
    def on_actionSave_triggered(self, arg):
        name = QtGui.QFileDialog.getSaveFileName(self)
        if QtCore.QFile.exists(name):
            with open(name, 'w') as file:
                text = self.ui.textEdit.toPlainText()
                file.write(text)
                file.close()

    @QtCore.pyqtSlot(bool)
    def on_actionExit_triggered(self, arg):
        choice = QtGui.QMessageBox.question(self, 'Exit', 'Are you sure?', QtGui.QMessageBox.Yes | QtGui.QMessageBox.No)
        if choice == QtGui.QMessageBox.Yes:
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
        except Exception, e:
            self.ui.statusbar.showMessage('Port ' + name + ' not available')
        if self.ser.isOpen():
            self.tim.start(100)
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
                        self.ui.lineEditRotAngle.setText(str(rotAngle))
                        self.ui.lineEditTiltAngle.setText(str(tiltAngle))
                        messageText = ''
                        if (data[6] & (1 << 0)):
                            messageText = messageText + 'Rotate finsih'
                        if (data[6] & (1 << 2)):
                            messageText = messageText + 'Rotate active'
                        if (data[6] & (1 << 4)):
                            messageText = messageText + 'Rotate error'
                        messageText = messageText + ' '    
                        if (data[6] & (1 << 1)):
                            messageText = messageText + 'Tilt finsih'
                        if (data[6] & (1 << 3)):
                            messageText = messageText + 'Tilt active'
                        if (data[6] & (1 << 5)):
                            messageText = messageText + 'Tilt error'
                        self.ui.statusbar.showMessage(messageText)
                    else:
                        self.ui.statusbar.showMessage('Unknown')

app = QtGui.QApplication(sys.argv)
main = MainWindow()
main.show()
sys.exit(app.exec_())
