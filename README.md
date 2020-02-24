# MrStitch

Network based Panorama project for CS176B

## How to run

### Install OpenCV library

If you are using Mac, plase check out [how to install opencv](https://medium.com/@jaskaranvirdi/setting-up-opencv-and-c-development-environment-in-xcode-b6027728003)


### Compile client and server

To compile server program: 

~~~~
g++ $(pkg-config --cflags --libs opencv4) -std=c++11  servername.cpp -o servername
~~~~

To compile client program: 

~~~~
g++ clientname.cpp -o clientname
~~~~

### Modify IP address

Open the client program, change the variavle *server_ip* to your server IP address.

