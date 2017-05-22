CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT("pre-reclaim" CXX)

SET(SRCS logger.cpp
	 testset.cpp
	 launcher.cpp
	 proc.cpp
	 cpu_usage.cpp
	 meminfo.cpp
         main.cpp )
SET(BINDIR ${CMAKE_INSTALL_PREFIX}/bin)

ADD_DEFINITIONS (-DTIZEN)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED aul
				json-glib-1.0)

FOREACH(flag ${pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

MESSAGE("### CMAKE_SYSTEM_VERSION :: ${EXTRA_CFLAGS} ###")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Werror-implicit-function-declaration")
SET(CMAKE_CXX_FLAGS_DEBUG "-O2 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

ADD_EXECUTABLE("run_test" ${SRCS})
TARGET_LINK_LIBRARIES("run_test" ${PKGS_LDFLAGS} "-laul -lbundle -ljson-glib-1.0 -lgobject-2.0 -lglib-2.0")

INSTALL(TARGETS "run_test" DESTINATION ${BINDIR})

