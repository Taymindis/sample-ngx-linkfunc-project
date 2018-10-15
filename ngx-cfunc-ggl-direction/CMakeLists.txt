cmake_minimum_required(VERSION 2.8)
project(cfun_ggldirectionProject)

###############################################################################
## file globbing ##############################################################
###############################################################################

# these instructions search the directory tree when cmake is
# invoked and put all files that match the pattern in the variables 
# `sources` and `data`
file(GLOB_RECURSE sources src/main/*.c src/main/*.cpp src/main/*.h src/main/*.hpp src/libjsoncpp/*.cpp src/libjsoncpp/*.inl src/libjsoncpp/*.in)
file(GLOB_RECURSE sources_test src/test/*.cpp src/libjsoncpp/*.cpp src/libjsoncpp/*.inl src/libjsoncpp/*.in)
file(GLOB_RECURSE data resources/*)
# you can use set(sources src/main.cpp) etc if you don't want to
# use globing to find files automatically


link_directories(/usr/local/lib /usr/lib)

###############################################################################
##################### target definitions ######################################
###############################################################################

# add the data to the target, so it becomes visible in some IDE
add_library(cfun_ggldirection SHARED ${sources} ${data})
set_property(TARGET cfun_ggldirection PROPERTY POSITION_INDEPENDENT_CODE ON)

if(NOT ${CMAKE_VERSION} LESS 3.2)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    message(STATUS "Checking compiler flags for C++11 support.")
    # Set C++11 support flags for various compilers
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
    check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
    if(COMPILER_SUPPORTS_CXX11)
        message(STATUS "C++11 is supported.")
        if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        endif()
    elseif(COMPILER_SUPPORTS_CXX0X)
        message(STATUS "C++0x is supported.")
        if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -stdlib=libc++")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
        endif()
    else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
endif()


# just for cfun_ggldirection add some compiler flags
target_compile_options(cfun_ggldirection PUBLIC -rdynamic -std=c++11)

# this lets me include files relative to the root src dir with a <> pair
target_include_directories(cfun_ggldirection PUBLIC src/main)

# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
file(COPY ${data} DESTINATION ${PROJECT_BINARY_DIR})

###############################################################################
## dependencies ###############################################################
###############################################################################

# this defines the variables Boost_LIBRARIES that contain all library names
# that we need to link to
# find_package(Boost 1.36.0 COMPONENTS filesystem system REQUIRED)
SET(LIBPATH /usr/lib /usr/local/lib)
include_directories(src/main src/includejsoncpp /usr/local/include /usr/include)

find_library(CURL_LIB NAMES curl PATHS ${LIBPATH})
if(CURL_LIB)
  message(STATUS "curl found at ${CURL_LIB}. curl support is compiled in.")
else(CURL_LIB)
  message( FATAL_ERROR "curl not found. Please install curl")
endif(CURL_LIB)

find_library(BACKCURL_LIB NAMES backcurl PATHS ${LIBPATH})
if(BACKCURL_LIB)
  message(STATUS "backcurl found at ${BACKCURL_LIB}. backcurl support is compiled in.")
else(BACKCURL_LIB)
  message( FATAL_ERROR "backcurl not found. Please install backcurl")
endif(BACKCURL_LIB)

find_library(THREAD_LIB NAMES pthread PATHS ${LIBPATH})
if(THREAD_LIB)  
else(THREAD_LIB)
  message( FATAL_ERROR "pthread not found. Please install pthread")
endif(THREAD_LIB)



###############################################################################
## FIND PROGRAM, sometimes you might need openssl for password encryption######
###############################################################################
find_program(OPENSSL "openssl")
IF(OPENSSL)
  message(STATUS "OPENSSL Available.")
else(OPENSSL)
  message( FATAL_ERROR "openssl is needed for this project password encryption. Please install openssl")
endif(OPENSSL)

target_link_libraries(cfun_ggldirection PUBLIC pthread curl backcurl)


###############################################################################
## testing ####################################################################
###############################################################################

# this is for our testing framework
# we don't add REQUIRED because it's just for testing
find_package(GTest)

if(GTEST_FOUND)
  message(STATUS "GTEST Available.")
  enable_testing()

  add_executable(unit_tests ${sources_test})

  # we add this define to prevent collision with the main
  # this might be better solved by not adding the source with the main to the
  # testing target
  target_compile_definitions(unit_tests PUBLIC UNIT_TESTS)

  # this allows us to use our executable as a link library
  # therefore we can inherit all compiler options and library dependencies
  set_target_properties(cfun_ggldirection PROPERTIES ENABLE_EXPORTS on)

  target_link_libraries(unit_tests PUBLIC
    ${GTEST_BOTH_LIBRARIES} curl backcurl pthread
    # cfun_ggldirection
  )

  target_include_directories(unit_tests PUBLIC
    ${GTEST_INCLUDE_DIRS} # doesn't do anything on Linux
  )

  add_test(cfuncggldir_test unit_tests)

endif()

###############################################################################
## packaging ##################################################################
###############################################################################

# all install commands get the same destination. this allows us to use paths
# relative to the executable.
install(TARGETS cfun_ggldirection DESTINATION installed_destination)
# this is basically a repeat of the file copy instruction that copies the
# resources in the build directory, but here we tell cmake that we want it
# in the package
install(DIRECTORY resources DESTINATION installed_destination)

# now comes everything we need, to create a package
# there are a lot more variables you can set, and some
# you need to set for some package types, but we want to
# be minimal here
set(CPACK_PACKAGE_NAME "cfun_ggldirection")
set(CPACK_PACKAGE_VERSION "1.0.0")

# we don't want to split our program up into several things
set(CPACK_MONOLITHIC_INSTALL 1)

# This must be last
include(CPack)
