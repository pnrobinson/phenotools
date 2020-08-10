project(libphenotools)

add_library(${PROJECT_NAME} STATIC
  base.pb.cc
  interpretation.pb.cc
  phenopackets.pb.cc
  edge.cc
  hpoannotation.cc
  jsonobo.cc
  myexception.cc
  ontology.cc
  phenotools.cc
  property.cc
  termid.cc
  ${PROTO_SRCS} ${PROTO_HDRS}
)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
target_link_libraries(libphenotools ${Protobuf_LIBRARIES})

add_library(libs::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(${PROJECT_NAME}
	PRIVATE
		cxx_std_17
)

add_subdirectory(tests)
