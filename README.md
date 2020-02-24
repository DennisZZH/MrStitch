# MrStitch

Network based Panorama project for CS176B

## How to run

### Install OpenCV library

### Compile client and server

To compile sequantial server: 

~~~~
g++ $(pkg-config --cflags --libs opencv4) -std=c++11  sequantial_server.cpp -o sequantial_server
~~~~

To compile parallel server with N threads:

~~~~
g++ $(pkg-config --cflags --libs opencv4) -std=c++11 parallel_server_N.cpp -o parallel_server_N
~~~~

To compile parallel server with K threads:

~~~~
g++ $(pkg-config --cflags --libs opencv4) -std=c++11 parallel_server_K.cpp -o parallel_server_K
~~~~

To compile the clients: 

~~~~
g++ clientname.cpp -o clientname
~~~~
