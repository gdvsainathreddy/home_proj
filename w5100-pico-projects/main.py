import usocket as socket
import uos
import time
import network

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

def get_mac_address():
    # Initialize the network interface
    spi = network.SPI(1)
    cs = machine.Pin(10)
    rst = machine.Pin(9)
    nic = network.WIZNET5K(spi, cs, rst, wiz_type=network.WIZNET5K.W5100S)
    return nic.mac_address

def serve_files(conn, addr):
    request = conn.recv(1024).decode()
    request_method = request.split(' ')[0]
    if request_method == 'GET':
        if request.split()[1] == '/files':
            file_list = ''
            for file in uos.listdir():
                file_list += f'<a href="/files/{file}">{file}</a>'
            response = HTML_TEMPLATE.format(file_list)
        else:
            response = 'HTTP/1.1 404 Not Found\r\n\r\n'
    elif request_method == 'POST':
        if '/upload' in request:
            content_length = int(request.split('Content-Length: ')[1].split('\r\n')[0])
            content = conn.recv(content_length)
            filename = content.split(b'filename="')[1].split(b'"')[0].decode()
            print("Filename: ",filename)
            if file_exists(filename):
                print("file exists")
                response = "File already exists. Overwrite? (Yes/No)"
                conn.sendall(response.encode())
                confirmation = conn.recv(1024).decode().strip().lower()
                if confirmation != 'yes':
                    return
            with open(filename, 'wb') as f:
                f.write(content)
            response = f"File {filename} uploaded successfully."
        else:
            response = 'HTTP/1.1 404 Not Found\r\n\r\n'
    else:
        response = 'HTTP/1.1 405 Method Not Allowed\r\n\r\n'

    conn.sendall(response.encode())
    conn.close()

def start_server():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    while True:
        try:
            s.bind(("192.168.0.22", 80))
            break
        except OSError as e:
            if e.args[0] == 98:
                print("Port 80 is in use, waiting...")
                print("5")
                time.sleep(1)
                print("4")
                time.sleep(1)
                print("3")
                time.sleep(1)
                print("2")
                time.sleep(1)
                print("1")
                time.sleep(1)
            else:
                raise
    s.listen(5)
    print("Server started at 192.168.0.22:80")
    #print("MAC Address:", get_mac_address())

    while True:
        conn, addr = s.accept()
        print("Connection from:", addr)
        serve_files(conn, addr)

start_server()
