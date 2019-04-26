Random.so: redismodule.h Random.cc
	g++ -std=c++17 -shared -o Random.so -fPIC Random.cc

all: Random.so

clean:
	rm -rf *.so
