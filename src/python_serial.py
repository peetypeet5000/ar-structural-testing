import time
import serial

serial_device = serial.Serial(
    port='/dev/ttyUSB0',
    baudrate=9600,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

serial_device.open()
serial_device.isOpen()

while 1:
    serial_output = ''
    while serial_device.inWaiting() > 0:
        serial_output += serial_device.read(1)
        if serial_output == '\r':
            print serial_output
