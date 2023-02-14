ALL: blocking_echo_server non_blocking_echo_server multiplexing_echo_server signal_driven_echo_server
.PHONY : ALL
multiplexing_echo_server:
	gcc multiplexing_echo_server.c -o multiplexing_echo_server
blocking_echo_server:
	gcc blocking_echo_server.c -lpthread -o blocking_echo_server
non_blocking_echo_server:
	gcc non_blocking_echo_server.c -o non_blocking_echo_server
signal_driven_echo_server:
	gcc signal_driven_echo_server.c -o signal_driven_echo_server
	
clean: 
	rm blocking_echo_server
	rm non_blocking_echo_server
	rm multiplexing_echo_server
	rm signal_driven_echo_server
