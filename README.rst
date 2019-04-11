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

  $ ./phenopacket_demo

It will input the Phenopacket included in the demo and output this. ::

  Phenopacket at: Gebbia-1997-ZIC3.json
  subject.id: III-1
  subject.age: 7W
  subject.sex: male
  id: HP:0002139: Arrhinencephaly
  id: HP:0001750: Single ventricle
  id: HP:0001643: Patent ductus arteriosus
  id: HP:0001746: Asplenia
  id: HP:0004971: Pulmonary artery hypoplasia
  id: HP:0001674: Complete atrioventricular canal defect
  id: HP:0001669: Transposition of the great arteries
  id: HP:0012890: Posteriorly placed anus
  id: HP:0001629: Ventricular septal defect
  id: HP:0012262: Abnormal ciliary motility
  id: HP:0004935: Pulmonary artery atresia
  id: HP:0003363: Abdominal situs inversus
