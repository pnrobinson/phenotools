project(libphenotools)

add_library(${PROJECT_NAME} STATIC
  base.pb.cc
  interpretation.pb.cc
  phenopackets.pb.cc
  myexception.cc
  property.cc
  edge.cc
  jsonobo.cc
  ontology.cc
  phenotools.cc
  termid.cc
  mondo.cc
  ${PROTO_SRCS} ${PROTO_HDRS}
)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
#include_directories(${CMAKE_CURRENT_BINARY_DIR})
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS foo.proto)
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS EXPORT_MACRO DLL_EXPORT foo.proto)
#protobuf_generate_python(PROTO_PY foo.proto)
#add_executable(bar bar.cc )
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
