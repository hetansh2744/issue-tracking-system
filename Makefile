################################################################################
# Executables
################################################################################

PROJECT = project
REST    = rest_server
GTEST   = test_${PROJECT}

################################################################################
# Compiler + Flags
################################################################################

CXX = g++
CXXVERSION = -std=c++17
CXXFLAGS = ${CXXVERSION} -g
CXXWITHCOVERAGEFLAGS = ${CXXFLAGS} -fprofile-arcs -ftest-coverage

# SQLite
SQLITE_PREFIX = third_party/sqlite-build

# Libraries
LINKFLAGS = -lgtest -lgmock -pthread \
	-L $(SQLITE_PREFIX)/lib \
	-Wl,-rpath,$(abspath $(SQLITE_PREFIX)/lib) \
	-lsqlite3

################################################################################
# Directories
################################################################################

SRC_DIR = src
MODEL_DIR = src/model
REPO_DIR = src/repo
VIEW_DIR = src/view
CONTROLLER_DIR = src/controller
SERVER_DIR = src/server
PROJECT_SRC_DIR = src/project

GTEST_DIR = test
SRC_INCLUDE = include

INCLUDE = -I ${SRC_INCLUDE} -I ${SQLITE_PREFIX}/include

################################################################################
# Tools
################################################################################

GCOV = gcov
LCOV = lcov
COVERAGE_RESULTS = results.coverage
COVERAGE_DIR = coverage
STATIC_ANALYSIS = cppcheck
STYLE_CHECK = cpplint
DESIGN_DIR = docs/design
DOXY_DIR = docs/code

################################################################################
# Source Groups
################################################################################

CORE_SRCS = \
	$(wildcard ${SRC_DIR}/*.cpp) \
	$(wildcard ${MODEL_DIR}/*.cpp) \
	$(wildcard ${REPO_DIR}/*.cpp) \
	$(wildcard ${VIEW_DIR}/*.cpp) \
	$(wildcard ${CONTROLLER_DIR}/*.cpp)

MAIN_SRC = ${PROJECT_SRC_DIR}/main.cpp

REST_SRCS = ${CORE_SRCS} \
	$(wildcard ${SERVER_DIR}/*.cpp)

################################################################################
# Default Target
################################################################################

.DEFAULT_GOAL := compileProject

################################################################################
# Clean
################################################################################

.PHONY: clean
clean:
	rm -rf *.gcov *.gcda *.gcno ${COVERAGE_RESULTS} ${COVERAGE_DIR}
	rm -rf docs/code/html
	rm -rf ${PROJECT} ${GTEST} ${REST} \
	       ${PROJECT}.exe ${GTEST}.exe ${REST}.exe
	rm -rf src/*.o src/model/*.o src/repo/*.o \
	       src/view/*.o src/controller/*.o \
	       src/server/*.o src/project/*.o
	rm -rf *~ \#* .\#* \
	src/*~ src/\#* src/.\#* \
	test/*~ test/\#* test/.\#* \
	include/*~ include/\#* include/.\#* \
	docs/design/*~ docs/design/\#* docs/design/.\#* \
	*.gcov *.gcda *.gcno

################################################################################
# Object Rule
################################################################################

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

################################################################################
# Build Targets
################################################################################

# Tests (no main)
${GTEST}: clean
	${CXX} ${CXXFLAGS} -o ./${GTEST} ${INCLUDE} \
	${GTEST_DIR}/*.cpp ${CORE_SRCS} ${LINKFLAGS}

# Project build (with main)
compileProject: clean
	${CXX} ${CXXVERSION} -o ${PROJECT} ${INCLUDE} \
	${CORE_SRCS} ${PROJECT_SRC_DIR}/*.cpp ${LINKFLAGS}

# REST server build
rest: clean
	${CXX} ${CXXVERSION} -o ${REST} ${INCLUDE} \
	${REST_SRCS} ${LINKFLAGS}

################################################################################
# Test Related Targets
################################################################################

all: ${GTEST} memcheck coverage docs static style

memcheck: ${GTEST}
	valgrind --tool=memcheck --leak-check=yes \
	--error-exitcode=1 ./${GTEST}

coverage: clean
	${CXX} ${CXXWITHCOVERAGEFLAGS} -o ./${GTEST} ${INCLUDE} \
	${GTEST_DIR}/*.cpp ${CORE_SRCS} ${LINKFLAGS}
	./${GTEST}
	${LCOV} --capture --gcov-tool ${GCOV} \
	--directory . --output-file ${COVERAGE_RESULTS} \
	--rc lcov_branch_coverage=1
	${LCOV} --extract ${COVERAGE_RESULTS} */*/*/${SRC_DIR}/* \
	-o ${COVERAGE_RESULTS}
	genhtml ${COVERAGE_RESULTS} --output-directory ${COVERAGE_DIR}

################################################################################
# Analysis + Style
################################################################################

static:
	${STATIC_ANALYSIS} --verbose --enable=all ${SRC_DIR} \
	${SRC_INCLUDE} --suppress=missingInclude \
	--suppress=useStlAlgorithm --error-exitcode=1

style:
	${STYLE_CHECK} ${SRC_DIR}/* ${GTEST_DIR}/* \
	${SRC_INCLUDE}/* \
	${MODEL_DIR}/* ${REPO_DIR}/* ${VIEW_DIR}/* \
	${CONTROLLER_DIR}/* ${SERVER_DIR}/* \
	${PROJECT_SRC_DIR}/*

################################################################################
# Docs
################################################################################

docs:
	doxygen ${DOXY_DIR}/doxyfile

################################################################################
# Run Targets
################################################################################

run:
	./${PROJECT}

run-rest:
	./${REST}
