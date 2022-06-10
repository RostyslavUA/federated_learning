The goal of this project is to enable vehicular cooperative perception using Deep Learning techniques in Federated Learning paradigm [[1]](#1). Technically speaking, this repository provides the modules to generate and manage a vehicular network in OMNeT++ [[2]](#2), collect the communication statistics in Veins [[3]](#3) (for now only bandwidth consumed by the message transmission and delay) as well as C++ - Python API [[4]](#4).
A block diagram of the program can be found below.

![image](https://user-images.githubusercontent.com/49762976/173147791-e93bc5ba-80a3-49c9-8c4e-9870f2ab0644.png)

Program versions used to run the simulation
- OMNeT++ 5.6.1
- Veins 5.2
- SUMO 1.11

It was executed on Ubuntu-20.04 via WSL
## References
<a id="1">[1]</a> https://ieeexplore.ieee.org/document/9473581 \
<a id="2">[2]</a> https://doc.omnetpp.org/workshop2008/omnetpp40-paper.pdf \
<a id="3">[3]</a> https://ieeexplore.ieee.org/document/5510240 \
<a id="4">[4]</a> https://zeromq.org/
