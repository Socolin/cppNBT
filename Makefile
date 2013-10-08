CXX	 = g++
CXXFLAGS = -Wall -ansi -pedantic -O3 -fno-elide-constructors -std=c++11
CPPFLAGS = -MMD
LDLIBS	 = -lz
LDFLAGS	 =
TEST_TARGET = nbttest
TARGET_LIB = libcppnbt.a

# Main program sources
FILES	 = util.cc tag.cc tag_byte.cc tag_byte_array.cc tag_compound.cc \
	   tag_list.cc tag_end.cc tag_double.cc tag_long.cc tag_string.cc \
	   tag_short.cc tag_int.cc tag_float.cc nbtfile.cc tag_int_array.cc \
	   nbtbuffer.cc

SOURCES=$(addprefix src/, ${FILES})
HEADERS=$(addsuffix .h, $(basename ${SOURCES}))
OBJECTS=$(addsuffix .o, $(basename ${SOURCES}))
DEPS=$(addsuffix .d, $(basename ${SOURCES}))

all: ${TEST_TARGET}

${TARGET_LIB}: ${OBJECTS}
	ar -rcs $@ $^

${TEST_TARGET}: test.cc ${TARGET_LIB}
	${LINK.cc} -o $@ $^ ${LDLIBS}

debug: CXXFLAGS+=-g3
debug: CXXFLAGS:=$(filter-out -O3, ${CXXFLAGS})
debug: all

clean:
	${RM} ${OBJECTS} ${DEPS} ${TEST_TARGET} ${TARGET_LIB}

-include ${DEPS}

