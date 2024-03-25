from usocket import socket
from machine import Pin,SPI
#import network
#import time
import json
#import usocket as socket
import uos
import time
import network
import machine
import _thread


led = Pin(25, Pin.OUT)

HTML_TEMPLATE = """<!DOCTYPE html>
<html>
<head><title>File Server</title></head>
<body>
<h2>Files:</h2>
<ul>{}</ul>
<form method="POST" action="/upload" enctype="multipart/form-data">
    <input type="file" name="file">
    <input type="submit" value="Upload">
</form>
</body>
</html>
"""


def file_exists(filename):
    try:
        uos.stat(filename)
        return True
    except OSError:
        return False
    
#W5x00 chip init
def w5x00_init():
    spi=SPI(0,2_000_000, mosi=Pin(19),miso=Pin(16),sck=Pin(18))
    nic = network.WIZNET5K(spi,Pin(17),Pin(20)) #spi,cs,reset pin
    nic.active(True)
    
    #None DHCP
    nic.ifconfig(('192.168.0.22','255.255.255.0','192.168.0.1','8.8.8.8'))
    
    #DHCP
    #nic.ifconfig('dhcp')
    print('IP address :', nic.ifconfig())
    
    while not nic.isconnected():
        time.sleep(1)
        print(nic.regs())

fan_status = {}
gpio_status = {}
lock_status = {}
rgb_status = {}

def set_response(client, json_type, response):
    client.send('HTTP/1.0 200 OK\r\n')
    if json_type:
        client.send('Content-type: application/json\r\n\r\n')
        client.sendall(response)
    else:
        client.send('\r\n')
        client.sendall(response)

def parse_path(path):
    return path.split('/')[1:]

def handle_request(client):
    try:
        request = client.recv(1024).decode('utf-8')
        path = parse_path(request.split(' ')[1])
        print(path)
        if path[0] == 'security' and len(path) > 2:
            response = "OK"
            set_response(client, json_type=False, response=response)
            return
        elif path[0].startswith("gpio") and len(path) > 2:
            pin = int(path[0][4:])
            print(pin)
            action = path[1]
            if action in ['on', 'off', 'status']:
                if action == 'on':
                    gpio_status[pin] = 1
                    response = "OK"
                    led.value(1)
                    # Add your GPIO handling code here
                elif action == 'off':
                    gpio_status[pin] = 0
                    response = "OK"
                    led.value(0)
                    # Add your GPIO handling code here
                elif action == 'status':
                    if pin not in gpio_status:
                        gpio_status[pin] = 0
                    response = str(gpio_status[pin])
                set_response(client, json_type=False, response=response)
                return
        response = {'status': 'error', 'message': 'Error from RPI'}
        # Add similar handling for other routes
        request_method = request.split(' ')[0]
        if request_method == 'GET':
            if request.split()[1] == '/files':
                file_list = ''
                for file in uos.listdir():
                    file_list += f'<li><a href="/files/{file}">{file}</a></li>'
                response = HTML_TEMPLATE.format(file_list)
            elif request.split()[1].startswith('/files/'):
                filename = request.split()[1][7:]
                if file_exists(filename):
                    with open(filename, 'rb') as f:
                        content = f.read()
                    response = 'HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Disposition: attachment; filename="{}"\r\n\r\n'.format(filename)
                    client.sendall(response.encode())
                    client.sendall(content)
                else:
                    response = 'HTTP/1.1 404 Not Found\r\n\r\n'
            else:
                response = 'HTTP/1.1 404 Not Found\r\n\r\n'
        elif request_method == 'POST':
            if '/upload' in request:
                content_length = int(request.split('Content-Length: ')[1].split('\r\n')[0])
                content = client.recv(content_length)
                filename = content.split(b'filename="')[1].split(b'"')[0].decode()
                print("Filename: ",filename)
                with open(filename, 'wb') as f:
                    f.write(content)
                if file_exists(filename):
                    response = f"File {filename} Replaced successfully."
                else:
                    response = f"File {filename} uploaded successfully."
            else:
                response = 'HTTP/1.1 404 Not Found\r\n\r\n'
        else:
            response = 'HTTP/1.1 405 Method Not Allowed\r\n\r\n'

        # If the URL doesn't match our pattern, serve a 404 response
        #client.send('HTTP/1.0 404 Not Found\r\n')
        #client.send('Content-type: application/json\r\n\r\n')
        #client.sendall(json.dumps(response).encode())
        client.sendall(response.encode())
    except Exception as e:
        print('Error:', e)


def main():
    w5x00_init()
    s = socket()
    import time

    while True:
        try:
            s.bind(('', 80))
            break
        except OSError as e:
            if e.args[0] == 98:
                print("Port 80 is in use, waiting...")
                time.sleep(5)
            else:
                raise
    s.listen(5)
    print("I'm Listening now...")

    while True:
        conn, addr = s.accept()
        handle_request(conn)
        conn.close()

if __name__ == "__main__":
    main()