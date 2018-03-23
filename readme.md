# NachOS : An Experimental Operating System

## "Not Another Completely Heuristic Operating System"

## Collaborators :
* Bhagya Sharma
* Linda Shih
* Angela To
* Frank Mezzatesta

## Install on the Lab Machines

To start and test:

```
$ git clone https://github.com/h8liu/nachos.git
$ cd nachos/code
$ make
$ cd threads
$ ./nachos
```

This should work on the ieng machines.

## Install on a 64-bit Ubuntu 

Nachos requires 32-bit headers and libraries. To install these on
a Ubuntu 14.4 LTS 64-bit:

```
$ sudo apt-get install gcc-multilib g++-multilib lib32stdc++6
```

(Not sure if this works with other versions of Ubuntu or Linux dists.)

