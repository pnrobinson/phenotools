==========
phenotools
==========

A C++ app for working with phenotype ontologies as well as the `GA4GH Phenopackets standard
<https://github.com/phenopackets/phenopacket-schema>`_.

Installing protoc
~~~~~~~~~~~~~~~~~
First, install the latest version of the protobuf library and compiler.

Download protobuf from https://github.com/protocolbuffers/protobuf/releases/.
At the time of this writing, the latest version was 3.8.0.

For these instructions (https://github.com/protocolbuffers/protobuf/blob/master/src/README.md).

On an Ubuntu system, one can install the requirements as follows. ::

  $ sudo apt-get install autoconf automake libtool curl make g++ unzip

Following this, download the latest source code, compile and install it. Briefly ::

  $ tar xvfz protobuf-cpp-3.8.0.tar.gz
  $ cd protobuf-3.8.0/
  $ ./configure
  $ make
  $ make check
  $ sudo make install
  $ sudo ldconfig # refresh shared library cache.

Installing rapidjson
~~~~~~~~~~~~~~~~~~~~
Phenotools uses the `rapidjson C++ JSON library <http://rapidjson.org/>`_ to parse (OBO) ontology files in JSON format.
See the rapidjson home page for details. Installation on *NIX systems and Mac probably will work as follows. ::

	$ git clone https://github.com/Tencent/rapidjson.git
	$ cd rapidjson
	$ mkdir build
	$ cd build
	$ cmake ..
	$ make
	$ make install


Building phenotools
~~~~~~~~~~~~~~~~~~~

Download phenopacket-schema
^^^^^^^^^^^^^^^^^^^^^^^^^^^

First, download the phenopacket-schema code. In the following, we download the code 
directly into the phenotools repository. If you dwnload it somewhere else, adjust the
path accordingly. ::

  git clone https://github.com/phenopackets/phenopacket-schema.git

Generate the C++ code from the phenopacket-schema protobuf files
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The build process first generates C++ code to represent the Phenopacket on the
basis of the protobuf file. ::

  PROTO_PATH='phenopacket-schema/src/main/proto'
  protoc --proto_path=${PROTO_PATH} --cpp_out=lib ${PROTO_PATH}/base.proto
  protoc --proto_path=${PROTO_PATH} --cpp_out=lib ${PROTO_PATH}/interpretation.proto
  protoc --proto_path=${PROTO_PATH} --cpp_out=lib ${PROTO_PATH}/phenopackets.proto

Build phenotools
^^^^^^^^^^^^^^^^

Following this, we can build phenotools using cmake. ::

  mkdir build
  cd build
  cmake ..

This will generate a static library file as well as unit testing code and an app called phenotools.

Running the unittests
^^^^^^^^^^^^^^^^^^^^^
To run the unit tests following the build, enter ::

  ./lib/tests/phenopacket_tests

Running the app
^^^^^^^^^^^^^^^

Following the build, the app ``phenotools`` is located in the ``app`` subdirectory. The following
command runs phenotools with the ``--help`` option to show available commands and arguments. ::

  ./app/phenotools --help


A setup script
^^^^^^^^^^^^^^

There is a setup script that works on linux systems only. Enter the following to perform
all steps of the build process starting from the download of the phenopackets-schema repository. ::

  $ ./setup.sh



Running the app
~~~~~~~~~~~~~~~
Phenotools is in a beta stage, but it can already do a few useful things.

phenopacket validation
~~~~~~~~~~~~~~~~~~~~~~
The software currently decodes a Phenopacket from JSON format, does some Q/C,
and outputs a summary of the phenopacket to the shell. To run it, enter ::

  $ ./phenotools phenopacket -p Gebbia-1997-ZIC3.json

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

  #### We identified 1 Q/C issue ####
  [ERROR] phenopacket id missing

In this case, our example phenopacket was missing the ``id`` element but otherwise
was well-formed.

hpo mode
~~~~~~~~

Phenotools can read the HPO JSON file (``hp.json``) and output descriptive statistics (``-s``, ``--stats``) and/or
perform quality assessment (``-q``,``--qc``).

To run it, download the ``hp.json`` file from the HPO `GitHub page <https://github.com/obophenotype/human-phenotype-ontology>`_, and
execute the following command. ::

   $ ./phenotools hpo -j hp.json [-s/--stats][-q/--qc]


mondo mode
~~~~~~~~~~

Phenotools will be able to analyze and work with other JSON-serialized obo ontologies including MONDO. This feature is still exerimental.
For the adventurous, download the ``mondo.json`` file from the MONDO `GitHub page <https://github.com/monarch-initiative/mondo>`_.

To run the app, enter, ::

   $ ./phenotools mondo -j mondo.json [-s/--stats][-q/--qc]