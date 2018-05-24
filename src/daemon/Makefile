.PHONY: clean reset_execution

demone: demone.c ../common/common.c ../common/syscalls_wrappers.c daemon.c daemon_socket.c daemon_common.c daemonize.c
	gcc -std=gnu90 demone.c ../common/common.c ../common/syscalls_wrappers.c daemon.c daemon_socket.c daemon_common.c daemonize.c -o demone

demone_con_lock: demone_con_lock.c
	gcc -std=gnu90 demone_con_lock.c -o demone_con_lock

clean:
	rm demone

reset_execution:
	 rm -f /tmp/SO_project.lock /tmp/percorso_file_log.txt  /tmp/SO_stats_fifo
	 @echo "Killing daemon with pid" `pidof demone`
	 kill `pidof demone`

prove_struct: prove_struct.c
	gcc prove_struct.c -o prove_struct
	./prove_struct

