#The dir where Arduino is installed
ARDUINO_DIR = /home/alec/.arduino_ide/arduino-1.8.2
#the dir where arduino-makefile is installed
ARDMK_DIR = /home/alec/.arduino_mk
#the tools that are bundeled with the arduino distro
#AVR_TOOLS_DIR = /usr
#the type of board that is being used
BOARD_TAG = mega
BOARD_SUB = atmega2560
#the port type that the board uses
MONITOR_PORT  = /dev/ttyACM0

ARDUINO_LIBS = Adafruit_Sensor DHT_sensor_library
ARCHITECTURE = avr

BOARDS_TXT = $(ARDUINO_DIR)/hardware/arduino/avr/boards.txt

include ../../.arduino_mk/Arduino.mk
