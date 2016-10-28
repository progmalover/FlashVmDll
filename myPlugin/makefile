PROG = pluginexample.dll

all:$(PROG)

CC        = g++
RM        = rm -rf

SRCS      = $(wildcard *.cpp)
OBJS      = $(SRCS:%.cpp=%.o)

CFLAGS    = -g -Wall -O0  -fPIC
DEFINE    = 
INCLUDES  = -I. -I/root/boost_1_57_0/
CFLAGS   += $(DEFINE)
CFLAGS   += $(INCLUDES)

LIBS     = -L /root/boost_1_57_0/stage/lib/ -lpthread -lrt -ldl -lboost_thread -lboost_system
LFLAGS   = -shared

#ar cr libtest.a test1.o test2.o
#nm -s libtest.a

%.o:%.cpp
	$(CC) -c $<  $(CFLAGS)
%.o:%.c
	$(CC) -c $<  $(CFLAGS)

$(PROG):$(OBJS)
	${RM} $(PROG)
	$(CC) ${LFLAGS} -o $(PROG) $(OBJS) $(LIBS)


clean:
	${RM} $(PROG) *.o *.log *.user *.ncb *.d *.suo
