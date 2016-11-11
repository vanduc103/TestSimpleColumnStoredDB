CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 -std=c++11 -lstdc++ -lsqlparser -L/usr/local/lib/ -I/home/duclv/work/sql-parser/src/ -I/usr/local/boost_1_61_0 -I/server

OBJS =		TestCpp.o Table.o Dictionary.o Column.o ColumnBase.o PackedArray.o Util.o Transaction.o $(patsubst %.o,server/%.o,ServerSocket.o Socket.o) server_main.o 

LIBS =		-L/usr/local/lib/ -lsqlparser

TARGET =	TestServer

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
