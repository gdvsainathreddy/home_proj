from usocket import socket
from machine import Pin, SPI
import json
import uos
import time
import network

# Initialize LED pin
led = Pin(25, Pin.OUT)

# Global dictionaries to store statuses
gpio_status = {}
lock_status = {}
rgb_status = {}

# HTML template for file server
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
    """Check if file exists."""
    try:
        uos.stat(filename)
        return True
    except OSError:
        return False


def w5x00_init():
    """Initialize W5x00 chip."""
    spi = SPI(0, 2_000_000, mosi=Pin(19), miso=Pin(16), sck=Pin(18))
    nic = network.WIZNET5K(spi, Pin(17), Pin(20))  # spi,cs,reset pin
    nic.active(True)

    # Static IP configuration
    nic.ifconfig(('192.168.0.22', '255.255.255.0', '192.168.0.1', '8.8.8.8'))

    print('IP address:', nic.ifconfig())

    while not nic.isconnected():
        time.sleep(1)
        print(nic.regs())


def set_response(client, json_type, response):
    """Set HTTP response."""
    client.send('HTTP/1.0 200 OK\r\n')
    if json_type:
        client.send('Content-type: application/json\r\n\r\n')
        client.sendall(response)
    else:
        client.send('\r\n')
        client.sendall(response)


def parse_path(path):
    """Parse URL path."""
    return path.split('/')[1:]


def handle_gpio_request(pin, action):
    """Handle GPIO requests."""
    if action == 'on':
        gpio_status[pin] = 1
        led.value(1)
    elif action == 'off':
        gpio_status[pin] = 0
        led.value(0)
    elif action == 'status':
        if pin not in gpio_status:
            gpio_status[pin] = 0
    return str(gpio_status[pin])


def handle_request(client):
    """Handle HTTP requests."""
    try:
        request = client.recv(1024).decode('utf-8')
        path = parse_path(request.split(' ')[1])

        if path[0] == 'security' and len(path) > 2:
            response = "OK"
            set_response(client, json_type=False, response=response)
            return
        elif path[0].startswith("gpio") and len(path) > 2:
            pin = int(path[0][4:])
            action = path[1]
            if action in ['on', 'off', 'status']:
                response = handle_gpio_request(pin, action)
                set_response(client, json_type=False, response=response)
                return
        elif path[0] == 'files':
            if len(path) == 1:  # List files
                file_list = ''
                for file in uos.listdir():
                    file_list += f'<li><a href="/files/{file}">{file}</a></li>'
                response = HTML_TEMPLATE.format(file_list)
                set_response(client, json_type=False, response=response)
                return
            elif len(path) == 2:  # Serve file
                filename = path[1]
                if file_exists(filename):
                    with open(filename, 'rb') as f:
                        content = f.read()
                    response = 'HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Disposition: attachment; filename="{}"\r\n\r\n'.format(filename)
                    client.sendall(response.encode())
                    client.sendall(content)
                    return
        # Handling for other routes
        # ...
        # Handle POST request for file upload
        elif request.startswith('POST /upload'):
            content_length = int(request.split('Content-Length: ')[1].split('\r\n')[0])
            content = client.recv(content_length)
            filename = content.split(b'filename="')[1].split(b'"')[0].decode()
            with open(filename, 'wb') as f:
                f.write(content)
            if file_exists(filename):
                response = f"File {filename} Replaced successfully."
            else:
                response = f"File {filename} uploaded successfully."
            set_response(client, json_type=False, response=response)
            return
    except Exception as e:
        print('Error:', e)


def main():
    """Main function."""
    w5x00_init()
    s = socket()

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
    print("Listening now...")

    while True:
        conn, addr = s.accept()
        handle_request(conn)
        conn.close()


if __name__ == "__main__":
    main()
