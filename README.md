# random
Redis module embeddings of C++ std pseudo-random generators

Compiling:
===

You will need g++ and C++11. 

Just do 

```
git clone git@github.com:CBaquero/random.git
```
This will creat a subdirectory named "random". Change into that directory and do 

```
make
```
1
This should work on macOS (until a more complete Makefile is provided) and produce a library "Random.so"

Loading into Redis:
===

If you start redis server from the directory that has "Random.so" it should be as easy as running redis client (from any place) and entering "module load Random.so" into the redis client command line. The reply should be "OK".

Generating randoms:
===

* Random Uniform Integers: In the redis command line you can create uniform random numbers in a given range by issuing "random.unif START END" while replacing these capital words for any suitable range of integer numbers. The mininum and maximum in the range are those for "double double" C types. 

* Exponential Distribution: In the command line use "random.exp" to ask for a random from an exponential distribution (of default lambda 1). You can control the lambda by supplying any real number as LAMBDA, in "random.exp LAMBDA". The exponentially distributed random numbers are returned encoded as strings. 
