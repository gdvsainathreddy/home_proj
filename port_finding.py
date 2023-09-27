import serial
import serial.tools.list_ports


def find_device(vendor_id):
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        print(p.device)
        #print(p.name)
        #print(p.description)
        print(p.hwid)
        print(p.vid)
        #print(p.pid)
        #print(p.serial_number)
        #print(p.location)
        #print(p.manufacturer)
        #print(p.product)
        #print(p.interface)
        print("----------------")
        if p.vid == vendor_id:
            return p.device
    return None

def open_serial_connection(port, baudrate=115200):
    ser = serial.Serial(port, baudrate)
    return ser

vendor_id = 9025
device_port = find_device(vendor_id)


if device_port:
    ser = open_serial_connection(device_port)
    print(f"Connected to {device_port}")
else:
    print("Device not found.")
