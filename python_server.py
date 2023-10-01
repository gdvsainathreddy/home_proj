from http.server import SimpleHTTPRequestHandler, HTTPServer
import serial
import serial.tools.list_ports
import json
from urllib.parse import urlparse, parse_qs

fan_status = {}
gpio_status = {}
lock_status = {}
rgb_status = {}

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
    global fan_status
    global gpio_status
    global lock_status
    global rgb_status
    def set_response(self, json_type, response):
        self.send_response(200)
        if json_type:
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
        else:
            self.end_headers()
            self.wfile.write(str(response).encode())
        return
    
    def do_GET(self):
        path = self.path.split('/')
        if path[1].startswith('gpio') and len(path) > 3:
            pin = int(path[1][4:])
            action = path[2]
            if action in ['on', 'off', 'status']:
                if action == 'on':
                    gpio_status[pin] = 1
                    response = "OK"
                    # ser.write(f'SET {pin} 1\n'.encode())
                elif action == 'off':
                    gpio_status[pin] = 0
                    response = "OK"
                    # ser.write(f'SET {pin} 0\n'.encode())
                elif action == 'status':
                    if pin not in gpio_status:
                        gpio_status[pin] = 0
                    response = gpio_status[pin]
                    # ser.write(f'GET {pin}\n'.encode())
                # response = ser.readline().decode().strip()
                self.set_response(json_type=False,response=response)
            return
        elif path[1].startswith('fan'):
            url = urlparse(self.path)
            params = parse_qs(url.query)
            # print(params)
            fan = int(path[1][3:])
            action = path[2]
            if 'value' in params:
                int_value = int(params['value'][0])
                action = action.split("?")[0]
                # print(action)
                if action in ['setState', 'setRotationSpeed', 'setRotationDirection']:
                    fan_status[fan][action] = int_value
                    # print(fan_status)
                    response = "OK"
                    self.set_response(json_type=False,response=response)
                    return
            elif action == "status":
                if fan not in fan_status:
                    fan_status[fan] = {"setState": 0, "setRotationSpeed": 0, "setRotationDirection": 0}
                response = { "currentState": fan_status[fan]["setState"], "rotationSpeed" : fan_status[fan]["setRotationSpeed"], "rotationDirection": fan_status[fan]["setRotationDirection"]}
                self.set_response(json_type=True,response=response)
                return
        elif path[1].startswith('rgb'):
            url = urlparse(self.path)
            params = parse_qs(url.query)
            # print(params)
            rgb = int(path[1][3:])
            action = path[2]
            if 'value' in params:
                int_value = params['value'][0]
                action = action.split("?")[0]
                # print(action,int_value)
                if action == 'status':
                    rgb_status[rgb][action] = int_value
                    # print(fan_status)
                    response = int_value
                    self.set_response(json_type=False,response=response)
                    return
                elif action == 'brightness':
                    rgb_status[rgb][action] = str(int_value)
                    # print(fan_status)
                    response = int_value
                    self.set_response(json_type=False,response=response)
                    return
                elif action == 'color':
                    int_value = params['value'][0]
                    # Ensure int_value is a 6-digit hex number
                    int_value = int_value.lstrip('#')
                    if len(int_value) != 6:
                        int_value = 'FFFFFF'  # Default to white if the format is incorrect or missing
                    else:
                        int_value = int_value.zfill(6)  # Ensure it's always 6 digits
                    hex_color = int_value
                    rgb_status[rgb][action] = hex_color
                    response = hex_color
                    self.set_response(json_type=False, response=response)
                    return
            elif action in ['status', 'brightness', 'color']:
                if rgb not in rgb_status:
                    # print("_________________________________")
                    rgb_status[rgb] = {"status": 0, "brightness": "0", "color": "FFFFFF"}
                # print(rgb_status)
                response = rgb_status[rgb][action]
                self.set_response(json_type=False,response=response)
                return
        elif path[1].startswith('lock'):
            url = urlparse(self.path)
            params = parse_qs(url.query)
            # print(params)
            lock = int(path[1][4:])
            action = path[2]
            if action == 'setState':
                int_value = int(params['value'][0])
                fan_status[lock][action] = int_value
                response = "OK"
                self.set_response(json_type=False,response=response)
                return
            elif action == "status":
                if lock not in lock_status:
                    lock_status[lock] = {"setState": 0}
                response = { "currentState": lock_status[lock]["setState"]}
                self.set_response(json_type=True,response=response)
                return
        print(self.path)
        # If the URL doesn't match our pattern, serve a 404 response
        self.send_response(404)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        json_response = {'status': 'error', 'message': 'Not Found'}
        self.wfile.write(json.dumps(json_response).encode())

def run(server_class=HTTPServer, handler_class=RequestHandler, port=80):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print(f"Serving on port {port}")
    httpd.serve_forever()

# Start the server
run()

