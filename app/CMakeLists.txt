project(phenotools)


find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS foo.proto)
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS EXPORT_MACRO DLL_EXPORT foo.proto)
#protobuf_generate_python(PROTO_PY foo.proto)
#add_executable(bar bar.cc )
#target_link_libraries(bar )

add_executable(${PROJECT_NAME}
	main.cpp ${PROTO_SRCS} ${PROTO_HDRS}
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		libphenotools
    ${Protobuf_LIBRARIES}
)


target_compile_features(${PROJECT_NAME}
	PRIVATE
		cxx_std_17
)
