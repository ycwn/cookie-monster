
ARDUINO_ROOT = /stuff/project/wemos/arduino-1.8.3
ARDUINO      = arduino

PACKAGE    = arduino
ARCH       = avr
BOARD      = leonardo
PARAMETERS =

PORT   =
SKETCH = cookie-monster.ino


export PATH:=$(PATH):$(ARDUINO_ROOT)

ifeq ($(PORT),)
PORT = $(realpath $(firstword $(sort $(wildcard /dev/serial/by-id/*Arduino*))))
endif



all: upload


upload:
	$(ARDUINO) --upload --board $(PACKAGE):$(ARCH):$(BOARD)$(PARAMETERS) --port $(PORT) --verbose $(SKETCH)

