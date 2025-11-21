################################################################################
# Revision History -- at the bottom of the document
################################################################################
# The targets in this file are used in .gitlab-ci.yml and the files created
# are found in the .gitignore
################################################################################

################################################################################
# Variable definitions
################################################################################

PROJECT = project
REST = rest_server
GTEST = test_${PROJECT}

CXX = g++
CXXVERSION = -std=c++17
CXXFLAGS = ${CXXVERSION} -g
CXXWITHCOVERAGEFLAGS = ${CXXFLAGS} -fprofile-arcs -ftest-coverage
LINKFLAGS= -lgtest -lgmock -pthread

SRC_DIR = src
MODEL_DIR = src/model
REPO_DIR = src/repo
VIEW_DIR = src/view
CONTROLLER_DIR = src/controller
SERVER_DIR = src/server
PROJECT_MAIN_DIR = src/project

GTEST_DIR = test
SRC_INCLUDE = include
INCLUDE = -I ${SRC_INCLUDE}

GCOV = gcov
LCOV = lcov
COVERAGE_RESULTS = results.coverage
COVERAGE_DIR = coverage
STATIC_ANALYSIS = cppcheck
STYLE_CHECK = cpplint
DESIGN_DIR = docs/design
DOXY_DIR = docs/code

################################################################################
# Source groups
################################################################################

CORE_SRCS = \
  $(wildcard ${SRC_DIR}/*.cpp) \
  $(wildcard ${MODEL_DIR}/*.cpp) \
  src/repo/IssueRepository.cpp \
  $(wildcard ${VIEW_DIR}/*.cpp) \
  $(wildcard ${CONTROLLER_DIR}/*.cpp)

MAIN_SRC = ${PROJECT_MAIN_DIR}/main.cpp

REST_SRCS = ${CORE_SRCS} \
  $(wildcard ${SERVER_DIR}/*.cpp)

################################################################################
# Default
################################################################################

.DEFAULT_GOAL := compileProject

################################################################################
# Clean
################################################################################

.PHONY: clean
clean:
	rm -rf *.gcov *.gcda *.gcno results.coverage coverage
	rm -rf docs/code/html
	rm -rf ${PROJECT} ${GTEST} ${PROJECT}.exe \
	       ${GTEST}.exe ${REST} ${REST}.exe
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
# Build rules
################################################################################

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

# Tests (no main)
${GTEST}: clean
	${CXX} ${CXXFLAGS} -o ./${GTEST} ${INCLUDE} \
	${GTEST_DIR}/*.cpp ${CORE_SRCS} ${LINKFLAGS}

# Project build (with main)
compileProject: clean
	${CXX} ${CXXVERSION} -o ${PROJECT} ${INCLUDE} \
	${SRC_DIR}/*.cpp ${PROJECT_SRC_DIR}/*.cpp

################################################################################
# REST
################################################################################

rest: clean
	${CXX} ${CXXVERSION} -o ${REST} ${INCLUDE} \
	${REST_SRCS} \
	${OATPP_LIBS}

################################################################################
# Extra targets
################################################################################

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

static:
	${STATIC_ANALYSIS} --verbose --enable=all ${SRC_DIR} \
	${SRC_INCLUDE} --suppress=missingInclude \
	--suppress=useStlAlgorithm --error-exitcode=1

style:
	${STYLE_CHECK} ${SRC_DIR}/* ${GTEST_DIR}/* \
	${SRC_INCLUDE}/* \
	${MODEL_DIR}/* ${REPO_DIR}/* ${VIEW_DIR}/* \
	${CONTROLLER_DIR}/* ${SERVER_DIR}/* \
	${PROJECT_MAIN_DIR}/*

docs:
	doxygen ${DOXY_DIR}/doxyfile

run:
	./${PROJECT}

run-rest:
	./${REST}

################################################################################
# Revision History
################################################################################
