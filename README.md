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
 
This should work on macOS (until a more complete Makefile is provided) and produce a library "Random.so"

Loading into Redis:
===

If you start redis server from the directory that has "Random.so" it should be as easy as running redis client (from any place) and entering 

```
module load Random.so
```

into the redis client command line. The reply should be "OK".

Generating randoms:
===

* Gaussian Distribution: This distribution has optional parameters for MEAN (default 0.0) and STDDEV (default 1.0). The command has the format

```
random.norm [MEAN] [STDDEV]
```

* Discrete Uniform Distribution: In the redis command line you can create uniform random integers in a given range by issuing 

```
random.dunif START END
```

while replacing these capital words for any suitable range of integer numbers. The mininum and maximum in the range are those for "long long" C types. Make sure that START <= END.

* Uniform Distribution: In the redis command line you can create uniform random real numbers in a given range by issuing 

```
random.unif START END
```

while replacing these capital words for any suitable range of real numbers. The mininum and maximum in the range are those for "double" C types. Make sure that START <= END.


* Exponential Distribution: In the command line use 

```
random.exp
```

to ask for a random from an exponential distribution (of default lambda 1). You can control the lambda by supplying any real number as LAMBDA, in 

```
random.exp [LAMBDA]
```
The exponentially distributed random numbers are returned encoded as strings. 

Storing multiple randoms:
===

The several distributions provide support for storing multiple randoms in a key as a Redis list structure. These variants have a name that append an "l" (from list) to the distribution name. The first two parameters are obligatory and indicate the KEY and COUNT. Examples:

* The following command stores 10 exponentially distributed random numbers in key "foo": 

```
random.lexp foo 10
```

* The following command generates and stores in key "bar" 100 random numbers that follow the height distribution of US women (in inches). 

```
random.lnorm bar 100 65 3.5
```

Histograms:
===

You can check the quality of your samples by getting its histogram. The optional CELLS parameter (default of 10) states the number of desired cells. 
If the optional COLUMNS parameter is provided, the reply will show ASCII bars for each cell filled with '*', and where COLUMNS states the widest bar.

```
random.hist KEY [CELLS] [COLUMNS]
```

Example:

```
127.0.0.1:6379> random.lnorm akey 10000
(integer) 10000
127.0.0.1:6379> random.hist akey 30 50
 1) ""
 2) ""
 3) "*"
 4) "*"
 5) "**"
 6) "*****"
 7) "*******"
 8) "**********"
 9) "*******************"
10) "***********************"
11) "*******************************"
12) "****************************************"
13) "****************************************"
14) "**************************************************"
15) "*********************************************"
16) "*********************************************"
17) "*****************************************"
18) "*********************************"
19) "******************************"
20) "*********************"
21) "**************"
22) "*********"
23) "*****"
24) "***"
25) "*"
26) "*"
27) ""
28) ""
29) ""
30) ""
```

