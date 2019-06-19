==========
phenotools
==========

A C++ app for working with the `GA4GH Phenopackets standard <https://github.com/phenopackets/phenopacket-schema>`_.



Installing protoc
~~~~~~~~~~~~~~~~~
First, install the latest version of the protobuf library and compiler.

Download protobuf from https://github.com/protocolbuffers/protobuf/releases/.
At the time of this writing, the latest version was 3.8.0.

For these instructions (https://github.com/protocolbuffers/protobuf/blob/master/src/README.md).

On an Ubuntu system, one can install the requirements as follows. ::

  $ sudo apt-get install autoconf automake libtool curl make g++ unzip

Following this, download the latest source code, compile and install it. Briefly ::

  $ ./configure
  $ make
  $ make check
  $ sudo make install
  $ sudo ldconfig # refresh shared library cache.


Building phenotools
~~~~~~~~~~~~~~~~~~~
The build process first generates C++ code to represent the Phenopacket on the
basis of the protobuf file. We use a Makefile to represent this dependency. Following
this, g++ is used to compile the code using the C++17 standard. Note that
the code will not compile with some older version of the protobuf library. Use
version 3.7.1 or later.

The ``setup.sh`` script downloads the GitHub repository, generates the C++ files from the
protobuf code, and compiles the ``phenotools`` executable.


This software can be built simply with ::

  $ ./setup.sh

Running the demo
~~~~~~~~~~~~~~~~
The software currently just decodes a Phenopacket from JSON format and outputs
some of the data. To run it, enter ::

  $ ./phenotools Gebbia-1997-ZIC3.json

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
  Gene: ZIC3[ENTREZ:7547]
	GRCh37: X:136649818C>T[]
	Disease: HETEROTAXY, VISCERAL, 1, X-LINKED; HTX1 [OMIM:306955]
  Metadata:
  Hpo Case Annotator : 1.0.13-SNAPSHOT(1970-01-01T00:00:00Z)
  human phenotype ontology: hp(HP;http://purl.obolibrary.org/obo/hp.owl;2018-03-08;http://purl.obolibrary.org/obo/HP_)
  Phenotype And Trait Ontology: pato(PATO;http://purl.obolibrary.org/obo/pato.owl;2018-03-28;http://purl.obolibrary.org/obo/PATO_)
  Genotype Ontology: geno(GENO;http://purl.obolibrary.org/obo/geno.owl;19-03-2018;http://purl.obolibrary.org/obo/GENO_)
  NCBI organismal classification: ncbitaxon(NCBITaxon;http://purl.obolibrary.org/obo/ncbitaxon.owl;2018-03-02;)
  Evidence and Conclusion Ontology: eco(ECO;http://purl.obolibrary.org/obo/eco.owl;2018-11-10;http://purl.obolibrary.org/obo/ECO_)
  Online Mendelian Inheritance in Man: omim(OMIM;https://www.omim.org;;)


Running the unittests
~~~~~~~~~~~~~~~~~~~~~

The setup script builds and runs an app called ``unittests`` that runs unit tests. This app can also be built
and run by the following commands. ::

  $ cd src/
  $ make unittests
  g++  -Wall -g -O0 --std=c++17 -I=. -pthread unittests.cpp base.pb.o interpretation.pb.o phenopackets.pb.o phenotools.o -o unittests -lprotobuf
  $ ./unittests 
  ===============================================================================
  All tests passed (111 assertions in 16 test cases)


