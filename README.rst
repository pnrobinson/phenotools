==========
phenotools
==========

A C++ app for working with the GA4GH Phenopackets standard.



Installation
~~~~~~~~~~~~
First, install the latest version of the protobuf library and compiler.

Download protobuf from https://github.com/protocolbuffers/protobuf/releases/.
At the time of this writing, the latest version was 3.7.1.

For these instructions (https://github.com/protocolbuffers/protobuf/blob/master/src/README.md)

Briefly

$ ./configure
$ make
$ make check
$ sudo make install
$ sudo ldconfig # refresh shared library cache.


Compiling this software
~~~~~~~~~~~~~~~~~~~~~~~
The build process first generates C++ code to represent the Phenopacket on the
basis of the protobuf file. We use a Makefile to represent this dependency. Following
this, g++ is used to compile the code using the C++17 standard. Note that
the code will not compile with some older version of the protobuf library. Use
version 3.7.1 or later.


This software can be built simply with ::

  $ make

Running the demo
~~~~~~~~~~~~~~~~
The software currently just decodes a Phenopacket from JSON format and outputs
some of the data. To run it, enter ::

  $ ./phenotools

We will soon improve the command line handling.
