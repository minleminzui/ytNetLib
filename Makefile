server:
	g++ util.cpp server.cpp Epoll.cpp InetAddress.cpp Socket.cpp -o server && \
	g++ util.cpp client.cpp -o client
clean:
	rm client && rm server