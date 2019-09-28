CC=gcc
CXX=g++
LOG_DIR=../../log
LIBS=-lpthread -L. -lslog
INC=-I$(LOG_DIR)
CFLAGS=-w -g -lpthread
CXXFLAGS=-w -g -std=c++11  -lpthread

all: libslog.so  slog_example
    
libslog.so: log.o
	$(CXX) $(CXXFLAGS)  -o $@ $<  -shared 
    
slog_example: main.o
	$(CXX)  $(CXXFLAGS) -o $@ $<   $(LIBS) 

main.o: main.cpp
	$(CXX) -c $(CXXFLAGS) -o $@  $< $(INC)

log.o: ../../log/log.cpp
	$(CXX) -c $(CXXFLAGS)  -o $@  $< -fPIC 
    
%.o:%.cpp
	$(CXX) -c $(CXXFLAGS)  -o $@ $< 

run:
	export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH && ./slog_example
    
.PHONY : clean
clean:
	rm -rf *.so *.o slog_example
