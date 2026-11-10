OPENCV   = /opt/homebrew/opt/opencv
CC       = clang
CXX      = clang++
CFLAGS   = -std=c11 -O2 -Wall
CXXFLAGS = -std=c++17 -O2 -Wall -I$(OPENCV)/include/opencv5
LDFLAGS  = -L$(OPENCV)/lib -Wl,-rpath,$(OPENCV)/lib
LDLIBS   = -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui \
           -lopencv_videoio -lopencv_objdetect -lopencv_dnn

facerec: main.o wajah.o
	$(CXX) main.o wajah.o -o facerec $(LDFLAGS) $(LDLIBS)

main.o: main.c wajah.h
	$(CC) $(CFLAGS) -c main.c -o main.o

wajah.o: wajah.cpp wajah.h
	$(CXX) $(CXXFLAGS) -c wajah.cpp -o wajah.o

clean:
	rm -f main.o wajah.o facerec