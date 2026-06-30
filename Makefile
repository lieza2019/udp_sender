#CC = gcc
CC = /usr/bin/gcc
CFLAGS = -Wall -g
#LD = gcc
LD = /usr/bin/gcc
LDFLAGS =

MV = mv
RM = rm
TOUCH = touch

udpsender : udpsender.o cmdopt_par.o
	$(LD) -o $@ $(LDFLAGS) $^

udpsender.o : udpsender.c udpsender.h
	$(CC) $(CFLAGS) -c $<
cmdopt_par.o : cmdopt_par.c udpsender.h
	$(CC) $(CFLAGS) -c $<

udpreceiv : udpreceiv.c
	$(CC) -o $@ $(CFLAGS) $^

.phony : clean
clean :
	-$(RM) ./*.o
	-$(RM) ./udpsender
	-$(RM) ./udpreceiv
	-$(RM) ./a.out
	-$(RM) ./*core
	-$(RM) ./*~
