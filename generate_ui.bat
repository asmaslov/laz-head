@echo off
IF EXIST ui_serial_test.py del /F ui_serial_test.py
pyuic4 serial_test.ui -o ui_serial_test.py
