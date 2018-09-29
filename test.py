import zmq

context = zmq.Context()
socket = context.socket(zmq.PAIR)
socket.connect("tcp://127.0.0.1:10201")
while True:
    socket.send_string("Hi")
    print(socket.recv())
