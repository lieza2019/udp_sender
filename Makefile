#CC = gcc
CC = /usr/bin/gcc
CFLAGS = -Wall -g
#LD = gcc
LD = /usr/bin/gcc
LDFLAGS =

MV = mv
RM = rm
TOUCH = touch

udpscat : udpscat.o cmdopt_par.o
	$(LD) -o $@ $(LDFLAGS) $^

udpscat.o : udpscat.c udpscat.h
	$(CC) $(CFLAGS) -c $<
cmdopt_par.o : cmdopt_par.c udpscat.h
	$(CC) $(CFLAGS) -c $<

.phony : clean
clean :
	-$(RM) ./*.o
	-$(RM) ./udpscat
	-$(RM) ./a.out
	-$(RM) ./*core
	-$(RM) ./*~
