project(phenotools_tests)

add_executable(${PROJECT_NAME}
    unittests.cpp
    ontology_test.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        libphenotools
)

target_compile_features(${PROJECT_NAME}
	PRIVATE
		cxx_std_17
)

set(CMAKE_BUILD_TYPE Debug)