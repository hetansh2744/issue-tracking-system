# Revision History -- at the bottom of the document
################################################################################
# The targets in this file are used in .gitlab-ci.yml and  the files created
# are found in the .gitignore
################################################################################
# Changing any names below can change the target names which will require that
# you update .gitlab_ci.yml and .gitignore
################################################################################

################################################################################
# Variable definitions
################################################################################

# Executable names
PROJECT = project
REST = rest_server
GTEST = test_${PROJECT}

# Compilation command and flags
CXX=g++
CXXVERSION= -std=c++17
CXXFLAGS= ${CXXVERSION} -g
CXXWITHCOVERAGEFLAGS = ${CXXFLAGS} -fprofile-arcs -ftest-coverage

# Oat++ flags
OATPP_LIBS = -loatpp -loatpp-swagger

LINKFLAGS= -lgtest -lgmock -pthread ${OATPP_LIBS}

# Directories
SRC_DIR = src
PROJECT_SRC_DIR = src/project
SERVER_SRC_DIR = src/server
DTOS_DIR = src/dtos
SERVICE_DIR = src/service
CONTROLLER_DIR = src/controller
GTEST_DIR = test
SRC_INCLUDE = include
INCLUDE = -I ${SRC_INCLUDE} -I src

# Tool variables
GCOV = gcov
LCOV = lcov
COVERAGE_RESULTS = results.coverage
COVERAGE_DIR = coverage
STATIC_ANALYSIS = cppcheck
STYLE_CHECK = cpplint
DESIGN_DIR = docs/design
DOXY_DIR = docs/code

################################################################################
# Default target
################################################################################

.DEFAULT_GOAL := compileProject

################################################################################
# Clean targets
################################################################################

.PHONY: clean-cov
clean-cov:
	rm -rf *.gcov *.gcda *.gcno ${COVERAGE_RESULTS} ${COVERAGE_DIR}

.PHONY: clean-docs
clean-docs:
	rm -rf docs/code/html

.PHONY: clean-exec
clean-exec:
	rm -rf ${PROJECT} ${GTEST} ${PROJECT}.exe \
	       ${GTEST}.exe ${REST} ${REST}.exe

.PHONY: clean-obj
clean-obj:
	rm -rf ${SRC_DIR}/*.o

.PHONY: clean-temp
clean-temp:
	rm -rf *~ \#* .\#* \
	${SRC_DIR}/*~ ${SRC_DIR}/\#* ${SRC_DIR}/.\#* \
	${GTEST_DIR}/*~ ${GTEST_DIR}/\#* ${GTEST_DIR}/.\#* \
	${SRC_INCLUDE}/*~ ${SRC_INCLUDE}/\#* ${SRC_INCLUDE}/.\#* \
	${PROJECT_SRC_DIR}/*~ ${PROJECT_SRC_DIR}/\#* ${PROJECT_SRC_DIR}/.\#* \
	${DESIGN_DIR}/*~ ${DESIGN_DIR}/\#* ${DESIGN_DIR}/.\#* \
	*.gcov *.gcda *.gcno

.PHONY: clean
clean: clean-cov clean-docs clean-exec clean-obj clean-temp

################################################################################
# Compilation targets
################################################################################

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

# Unit tests build
${GTEST}: ${GTEST_DIR} ${SRC_DIR} clean-exec
	${CXX} ${CXXFLAGS} -o ./${GTEST} ${INCLUDE} \
	${GTEST_DIR}/*.cpp ${SRC_DIR}/*.cpp ${LINKFLAGS}

# Normal project build
compileProject: ${SRC_DIR} ${PROJECT_SRC_DIR} clean-exec
	${CXX} ${CXXVERSION} -o ${PROJECT} ${INCLUDE} \
	${SRC_DIR}/*.cpp ${PROJECT_SRC_DIR}/*.cpp

################################################################################
# REST Server build (Oat++)
################################################################################

rest: ${SRC_DIR} ${PROJECT_SRC_DIR} clean-exec
	${CXX} ${CXXVERSION} -o ${REST} ${INCLUDE} \
	${SRC_DIR}/*.cpp \
	${PROJECT_SRC_DIR}/*.cpp \
	${SERVER_SRC_DIR}/*.cpp \
	${DTOS_DIR}/*.cpp \
	${SERVICE_DIR}/*.cpp \
	${CONTROLLER_DIR}/*.cpp \
	${OATPP_LIBS}

################################################################################
# Test targets
################################################################################

all: ${GTEST} memcheck coverage docs static style

memcheck: ${GTEST}
	valgrind --tool=memcheck --leak-check=yes \
	--error-exitcode=1 ./${GTEST}

coverage: clean-exec clean-cov
	${CXX} ${CXXWITHCOVERAGEFLAGS} -o ./${GTEST} ${INCLUDE} \
	${GTEST_DIR}/*.cpp ${SRC_DIR}/*.cpp ${LINKFLAGS}
	./${GTEST}
	${LCOV} --capture --gcov-tool ${GCOV} \
	--directory . --output-file ${COVERAGE_RESULTS} \
	--rc lcov_branch_coverage=1
	${LCOV} --extract ${COVERAGE_RESULTS} */*/*/${SRC_DIR}/* \
	-o ${COVERAGE_RESULTS}
	genhtml ${COVERAGE_RESULTS} --output-directory ${COVERAGE_DIR}
	make clean-temp

################################################################################
# Static + Style
################################################################################

static: ${SRC_DIR}
	${STATIC_ANALYSIS} --verbose --enable=all ${SRC_DIR} \
	${SRC_INCLUDE} --suppress=missingInclude \
	--suppress=useStlAlgorithm --error-exitcode=1

style: ${SRC_DIR} ${GTEST_DIR} ${SRC_INCLUDE} ${PROJECT_SRC_DIR}
	${STYLE_CHECK} ${SRC_DIR}/* ${GTEST_DIR}/* \
	${SRC_INCLUDE}/* ${PROJECT_SRC_DIR}/*

################################################################################
# Documentation
################################################################################

.PHONY: docs
docs: ${SRC_INCLUDE}
	doxygen ${DOXY_DIR}/doxyfile

.PHONY: version
version:
	doxygen --version
	cppcheck --version
	cpplint --version
	gcc --version
	gcov --version
	lcov --version
	valgrind --version

################################################################################
# Run helpers
################################################################################

run:
	./${PROJECT}

run-rest:
	./${REST}

################################################################################
# Revision History
################################################################################
