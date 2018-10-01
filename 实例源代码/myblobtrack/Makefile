OBJECT=myblobtrack

CXX=g++
CPPFLAGS=`pkg-config --cflags opencv`
LDFLAGS=`pkg-config --libs opencv`

all: $(OBJECT)

$(OBJECT): blobtrack.o myfgdetector.o
	$(CXX) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECT) *.o
