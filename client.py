import socket
import struct
import argparse


def recv_exact(sock: socket.socket, n: int) -> bytes:
    data = b''
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        if len(chunk) == 0:
            raise ConnectionError('EOF from server')
        data += chunk
    return data

def send_frame(sock: socket.socket, payload: bytes) -> None:
    header = struct.pack('!I', len(payload))
    sock.sendall(header + payload)

def recv_frame(sock: socket.socket) -> bytes:
    header = recv_exact(sock, 4)
    (length,) = struct.unpack('!I', header)
    if length > 10_000_000:
        raise ValueError('Message length is too long')
    return recv_exact(sock, length)

def main():
    parser = argparse.ArgumentParser(description='NOT(Redis) client')
    parser.add_argument('-H', '--host', type=str, required=False, default='127.0.0.1', help='Server host')
    parser.add_argument('-P', '--port', type=int, required=False, default=6379, help='Server port')
    parser.add_argument('-M', '--message', type=str, required=False, default='hello', help='Message to send')
    args = parser.parse_args()

    try:
        with socket.create_connection((args.host, args.port)) as sock:
            send_frame(sock, args.message.encode())
            response = recv_frame(sock)
            print('Message sent:', args.message)
            print('Server says:', response.decode("utf-8", errors='replace'))
    except ConnectionRefusedError as e:
        print('Connection refused by server:', e)
        exit(1)
    except ConnectionError as e:
        print('Failed to connect to server:', e)
        exit(1)
    except Exception as e:
        print('Unexpected error:', e)
        exit(1)
    
if __name__ == '__main__':
    main()
