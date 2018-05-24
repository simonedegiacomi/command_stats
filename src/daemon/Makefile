
demone: demone.c ../common/common.c ../common/common.h ../common/syscalls_wrappers.c ../common/syscalls_wrappers.h
	gcc -std=gnu90 demone.c ../common/common.c ../common/syscalls_wrappers.c -o demone

demone_con_lock: demone_con_lock.c
	gcc -std=gnu90 demone_con_lock.c -o demone_con_lock

clean:
	rm demone

prove_struct: prove_struct.c
	gcc prove_struct.c -o prove_struct
	./prove_struct

