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


PROTO_PATH='phenopacket-schema/src/main/proto'



## Generate the C++ files with protoc
if [ -e 'src/base.pb.h' ]; then
    echo "base.pb.h previously generated"
else
    protoc --proto_path=${PROTO_PATH} --cpp_out=src ${PROTO_PATH}/base.proto
fi

if [ -e 'src/interpretation.pb.h' ]; then
    echo "interpretation.pb.h previously generated"
else
    protoc --proto_path=${PROTO_PATH} --cpp_out=src ${PROTO_PATH}/interpretation.proto
fi

if [ -e 'src/phenopackets.pb.h' ]; then
    echo "phenopackets.pb.h previously generated"
else
    protoc --proto_path=${PROTO_PATH} --cpp_out=src ${PROTO_PATH}/phenopackets.proto
fi

   ${PROTO_PATH}/phenopackets.proto ${PROTO_PATH}/interpretation.proto


echo "Building phenotools..."

cd src; make
mv src/phenotools .








