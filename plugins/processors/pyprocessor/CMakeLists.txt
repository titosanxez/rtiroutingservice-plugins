#
# (c) 2018 Copyright, Real-Time Innovations, Inc.  All rights reserved.
#
#  RTI grants Licensee a license to use, modify, compile, and create derivative
#  works of the Software.  Licensee has the right to distribute object form
#  only for use with RTI products.  The Software is provided "as is", with no
#  warranty of any type, including any warranty for fitness for any purpose.
#  RTI is under no obligation to maintain or support the Software.  RTI shall
#  not be liable for any incidental or consequential damages arising out of the
#  use or inability to use the software.


cmake_minimum_required(VERSION 3.10)
project (ShapesProcessor)

# Find RTI Connext dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CONNEXTDDS_DIR}/resource/cmake")
find_package(
    RTIConnextDDS "6.0.1"
    REQUIRED
    COMPONENTS
        core
        routing_service
    )


find_package(PythonLibs 3.5 REQUIRED)

# Define the library that will provide the  Processor plugin
add_library(rtirspyprocessor
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NativeUtils.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyDynamicData.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PySamples.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyInput.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyOutput.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyRoute.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyProcessor.cxx")

# Set target properties for lang requirement output library name
set_target_properties(rtirspyprocessor
    PROPERTIES
        CXX_STANDARD 11
        OUTPUT_NAME_DEBUG rtirspyprocessord
        LIBRARY_OUTPUT_DIRECTORY "${output_dir}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${output_dir}"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${output_dir}"
        ARCHIVE_OUTPUT_DIRECTORY "${output_dir}"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${output_dir}"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${output_dir}"
        RUNTIME_OUTPUT_DIRECTORY "${output_dir}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${output_dir}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${output_dir}"
)

target_link_libraries(rtirspyprocessor
    RTIConnextDDS::cpp2_api
    RTIConnextDDS::routing_service_infrastructure
    ${PYTHON_LIBRARIES})

target_include_directories(rtirspyprocessor
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${PYTHON_INCLUDE_DIRS}"
)
