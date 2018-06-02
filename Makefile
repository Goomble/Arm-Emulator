PROGS = sum_array_a find_max_a \
        fib_iter_a fib_rec_a \
        find_str_a armemu 

OBJS = sum_array_a.o find_max_a.o \
       fib_iter_a.o fib_rec_a.o \
       find_str_a.o

all : ${PROGS}

sum_array_a : sum_array_a.s
	as -o sum_array_a.o sum_array_a.s

find_max_a : find_max_a.s
	as -o find_max_a.o find_max_a.s

fib_iter_a : fib_iter_a.s
	as -o fib_iter_a.o fib_iter_a.s

fib_rec_a : fib_rec_a.s
	as -o fib_rec_a.o fib_rec_a.s

find_str_a: find_str_a.s
	as -o find_str_a.o find_str_a.s

armemu : armemu.c ${OBJS} 
	gcc -o armemu armemu.c ${OBJS}

test : armemu
	./armemu

clean:
	rm -rf armemu ${OBJS}