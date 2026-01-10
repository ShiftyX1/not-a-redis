import socket
import struct
import argparse


def print_logo():
    print(r"""
╔════════════════════════════════════════════════════════════╗
║             _             _____  ______ _____ _____  _____ ║
║            | |           |  __ \|  ____|  __ \_   _|/ ____|║
║ _ __   ___ | |_    __ _  | |__) | |__  | |  | || | | (___  ║
║| '_ \ / _ \| __|  / _` | |  _  /|  __| | |  | || |  \___ \ ║
║| | | | (_) | |_  | (_| | | | \ \| |____| |__| || |_ ____) |║
║|_| |_|\___/ \__|  \__,_| |_|  \_\______|_____/_____|_____/ ║
╚════════════════════════════════════════════════════════════╝
""")

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
    print_logo()
    parser = argparse.ArgumentParser(description='NOT(Redis) client')
    parser.add_argument('-H', '--host', type=str, required=False, default='127.0.0.1', help='Server host')
    parser.add_argument('-P', '--port', type=int, required=False, default=6379, help='Server port')
    parser.add_argument('-M', '--message', type=str, required=False, default=None, help='Message to send (if not provided, starts interactive shell)')
    args = parser.parse_args()

    try:
        with socket.create_connection((args.host, args.port)) as sock:
            if args.message:
                send_frame(sock, args.message.encode())
                response = recv_frame(sock)
                print('Message sent:', args.message)
                print('Server says:', response.decode("utf-8", errors='replace'))
            else:
                print(f"Connected to {args.host}:{args.port}")
                print("Type 'quit' or 'exit' to leave.")
                import readline

                while True:
                    try:
                        prompt = f"{args.host}:{args.port}> "
                        cmd_line = input(prompt).strip()
                        
                        if not cmd_line:
                            continue
                            
                        if cmd_line.lower() in ('quit', 'exit'):
                            break
                        
                        send_frame(sock, cmd_line.encode())
                        response = recv_frame(sock)
                        print(response.decode("utf-8", errors='replace'))
                        
                    except KeyboardInterrupt:
                        print("\nType 'quit' or 'exit' to leave.")
                    except EOFError:
                        break

    except ConnectionRefusedError as e:
        print('Connection refused by server:', e)
        exit(1)
    except ConnectionError as e:
        print('Connection error:', e)
        exit(1)
    except Exception as e:
        print('Unexpected error:', e)
        exit(1)
    
if __name__ == '__main__':
    main()
