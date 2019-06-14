## Script to make it a little easier to run phenotools
# 1. Check that protoc is installed
# 2. Download the latest phenopackets version
# 3. create the generated C++ code
# 4. make the phenotools exectuable

# Not portable, will not work on Windows (sorry)

#1 protoc does not seem to work with pkg-config
# therefore, the following hack
LOC=`whereis libprotoc`
# if not found, we would see libprotoc:
NOTFOUND='libprotoc:'
# if it is found, then we should see something like
# ibprotoc: /usr/local/lib/libprotoc.a /usr/local/lib/libprotoc.la ...
L1=${#NOTFOUND}

if [ ${#LOC} \> $L1 ];
then
    echo "[INFO] protoc library found"
    echo
else
    echo "[ERROR] could not find protoc library, terminating script"
fi


#2 download the phenopackets repository
#  note if you want a more recent version, you must delete the current one


if [ -d 'phenopacket-schema' ]; then
    echo "[INFO] found phenopacket-schema directory. "
    echo "[INFO] delete it to download a newer version"
    echo
else
    git clone https://github.com/phenopackets/phenopacket-schema.git
fi

#3 create the C++ code
GENERATED_CPP_DIR='phenopacket-schema/target/generated-sources/protobuf/cpp'

if [ ! -d $GENERATED_CPP_DIR ]; then
    cd phenopacket-schema; mvn compile; mvn package
else
    echo "[INFO] C++ sources already present"
    echo
fi


# For simplicity, just check one of the generated files.
if [ ! -e ${GENERATED_CPP_DIR}/interpretation.pb.h ]; then
    cp ${GENERATED_CPP_DIR}/*.h src/.
    cp ${GENERATED_CPP_DIR}/*.cc src/.
else
    echo "[INFO] Already copied generated C++ source files"
    echo
fi


# Always compile a new version
if [ -e phenotools ]; then
    - rm phenotools
fi


echo "Building phenotools..."

cd gensrc; make
mv gensrc/phenotools .


make







