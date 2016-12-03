from threading import Thread
import socket

def sendall(sock, message):
	print "SERVER: trying to send "+message
	l = len(message+'\0')
	print "SERVER: sending length of {}".format(l)
	sock.send(str(l))
	print "SERVER: sent length of {}".format(l)
	a = int(sock.recv(11)[:-1])
	if a != l:
		print "SERVER: the hell is going on? sendall failed!"
		return -1
	print "SERVER: sending "+message
	sock.send(message+'\0')
def recvall(sock):
	l = int(sock.recv(11).strip('\0'))
	print "SERVER: recived length of {}".format(l)
	sock.send(str(l)+'\0')
	message = sock.recv(l)
	print "SERVER: recived "+message
	return message[:-1]

port = 6423
server_socket = socket.socket()
unp_file = open('unp','r')
unp_lst = unp_file.read().split('\n')
if '' in unp_lst:
	unp_lst.remove('')
unp_file.close()
while True: # Take the first free port after 80 [if it's not free]:
    try:
        server_socket.bind(('0.0.0.0', port))
        break
    except:
        port+=1
server_socket.listen(6)
print "SERVER: Running... on port {}".format(port) # -For The Record-

while True:
    c_sock, client_addr = server_socket.accept();    print "SERVER: a client accepted" # -For The Record-
    sendall(c_sock, "Connection established. Welcome!")
    unp = recvall(c_sock)
    print "SERVER: got username and password:\n    {}".format(unp)
    print "SERVER: current list: {}".format(unp_lst)
    if(unp not in unp_lst):
    	sendall(c_sock, 'N')
    	c_sock.close()
    else:
    	sendall(c_sock, 'Y')
    	while True:
    		req_num = int(recvall(c_sock))
    		if(req_num==1):
    			pass
    		elif(req_num==2):
    			pass
    		elif(req_num==3):
    			pass
    		elif(req_num==4):
    			break
    		elif(req_num==5):
    			pass
    		else:
    			print "SERVER: shit's fucked up! the hell is this number?"
    			pass
    c_sock.close()