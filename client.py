import shlex
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

def send_command(sock: socket.socket, text: str) -> None:
    parts = shlex.split(text)
    if not parts:
        return
    
    chunks = []
    
    n_args = len(parts)
    chunks.append(struct.pack('!I', n_args))
    
    for part in parts:
        part_bytes = part.encode()
        chunks.append(struct.pack('!I', len(part_bytes)))
        chunks.append(part_bytes)
        
    payload = b''.join(chunks)
    
    sock.sendall(struct.pack('!I', len(payload)) + payload)

def recv_response(sock: socket.socket) -> str:
    header = recv_exact(sock, 4)
    (length,) = struct.unpack('!I', header)
    if length > 10_000_000:
        raise ValueError('Message length is too long')
    
    body = recv_exact(sock, length)
    if length < 4:
         raise ValueError("Response too short")
         
    (status,) = struct.unpack('!I', body[:4])
    msg = body[4:]
    
    if status == 0:
        if not msg:
            return "(ok)"
        return msg.decode("utf-8", errors='replace')
    elif status == 2:
        return "(nil)"
    else:
        return f"(err) {msg.decode('utf-8', errors='replace')}"

def main():
    print_logo()
    parser = argparse.ArgumentParser(description='NOT(Redis) client')
    parser.add_argument('-H', '--host', type=str, required=False, default='127.0.0.1', help='Server host')
    parser.add_argument('-P', '--port', type=int, required=False, default=6379, help='Server port')
    parser.add_argument('-M', '--message', type=str, required=False, default=None, help='Command to send (e.g. "set k v")')
    args = parser.parse_args()

    try:
        with socket.create_connection((args.host, args.port)) as sock:
            if args.message:
                send_command(sock, args.message)
                response = recv_response(sock)
                print(response)
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
                        
                        send_command(sock, cmd_line)
                        response = recv_response(sock)
                        print(response)
                        
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
