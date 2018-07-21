INCLUDES = /home/hzmct/YKT/YKT/inc
LIBS     = /home/hzmct/YKT/YKT/lib
CC = /home/hzmct/gcc-arm-linux/bin/arm-linux-gnueabihf-gcc-4.9.2
XX = g++
CFLAGS = -Wall -O -g
CFLAGS = -O -I$(INCLUDES) -L$(LIBS)
CTD = -I$(INCLUDES) -L$(LIBS)
TARGET = rfid
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 
%.o:%.cpp
	$(XX) $(CFLAGS) -c $< -o $@ 
SOURCES = $(wildcard *.c ./src/*.c)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
$(TARGET) : $(OBJS)
	$(CC) $(OBJS) $(CTD) -o $(TARGET) -lpthread -ldb-6.2 -lmad -lcurl -lm -g -rdynamic 
#	chmod a+x $(TARGET)
clean:
	rm -rf $(wildcard rfid *.o *.log ./src/*.o)
