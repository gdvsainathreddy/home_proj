from http.server import SimpleHTTPRequestHandler, HTTPServer
import serial
import serial.tools.list_ports


def find_device(vendor_id):
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        print(p.vid)
        if p.vid == vendor_id:
            return p.device
    return None

def open_serial_connection(port, baudrate=115200):
    ser = serial.Serial(port, baudrate)
    return ser

vendor_id = 6790
device_port = find_device(vendor_id)


if device_port:
    ser = open_serial_connection(device_port)
    print(f"Connected to {device_port}")
else:
    print("Device not found.")


if device_port:
    ser = open_serial_connection(device_port)
    print(f"Connected to {device_port}")
else:
    print("Device not found.")

# Initialize serial connection
# ser = serial.Serial('/dev/ttyACM1', 115200)  # Changed baud rate to 115200, adjust port as needed

class RequestHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        path = self.path.split('/')
        if path[1].startswith('gpio') and len(path) > 3:
            pin = int(path[1][4:])
            action = path[2]
            if action in ['on', 'off', 'status']:
                if action == 'on':
                    ser.write(f'SET {pin} 1\n'.encode())
                elif action == 'off':
                    ser.write(f'SET {pin} 0\n'.encode())
                elif action == 'status':
                    ser.write(f'GET {pin}\n'.encode())
                response = ser.readline().decode().strip()
                self.send_response(200)
                self.end_headers()
                self.wfile.write(response.encode())
                return
        # If the URL doesn't match our pattern, serve a 404 response
        self.send_response(404)
        self.end_headers()
        self.wfile.write(b'Not Found')

def run(server_class=HTTPServer, handler_class=RequestHandler, port=80):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print(f"Serving on port {port}")
    httpd.serve_forever()

# Start the server
run()

