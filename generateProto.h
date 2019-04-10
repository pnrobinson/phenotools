#! /bin/sh
PROTO_PATH=/Users/peterrobinson/IdeaProjects/phenopacket-schema/src/main/proto/
PHENOPACKET=${PROTO_PATH}org/phenopackets/schema/v1/phenopackets.proto
BASE=${PROTO_PATH}org/phenopackets/schema/v1/core/base.proto
protoc --proto_path=${PROTO_PATH} --cpp_out=build ${PHENOPACKET} ${BASE}
