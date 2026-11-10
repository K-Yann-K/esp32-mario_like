import socket, sys, time
from pynput import keyboard

IP = sys.argv[1] if len(sys.argv) > 1 else "192.168.4.1"

TOUCHES = {
    keyboard.Key.left: "G",  "q": "G",
    keyboard.Key.right: "D", "d": "D",
    keyboard.Key.up: "S",    "z": "S",
}

s = socket.create_connection((IP, 5000), timeout=5)
s.settimeout(None)
s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
appuyees = set()

def cle(key):
    return key.char.lower() if getattr(key, "char", None) else key

def lettre_active(l):
    return any(TOUCHES.get(k) == l for k in appuyees)

def on_press(key):
    k = cle(key)
    l = TOUCHES.get(k)
    if l is None or k in appuyees:
        return
    deja = lettre_active(l)
    appuyees.add(k)
    if not deja:
        s.sendall(l.encode())

def on_release(key):
    k = cle(key)
    l = TOUCHES.get(k)
    if l is None or k not in appuyees:
        return
    appuyees.discard(k)
    if not lettre_active(l):
        s.sendall(l.lower().encode())

keyboard.Listener(on_press=on_press, on_release=on_release).start()

try:
    while True:
        data = s.recv(1024)
        if not data:
            print("ESP32 déconnecté")
            break
        sys.stdout.write(data.decode(errors="replace"))
        sys.stdout.flush()
except KeyboardInterrupt:
    s.close()