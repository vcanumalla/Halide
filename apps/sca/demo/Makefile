HALIDE_SRC_PATH = /Users/vcanumalla/projects/Halide/
HALIDE_BUILD_PATH = /Users/vcanumalla/projects/build/Halide/
HALIDE_INCLUDE_PATH = $(HALIDE_BUILD_PATH)/include
HALIDE_LIB_PATH = $(HALIDE_BUILD_PATH)/src

CODEGEN_LVL = 1

all: pipeline_gen static_lib_gen metrics_gen driver_gen

pipeline_gen: halide_sca_tutorial.cpp
	g++ -std=c++17 -I $(HALIDE_INCLUDE_PATH) -L $(HALIDE_LIB_PATH) -lHalide -lpthread -ldl -o pipeline.out halide_sca_tutorial.cpp

static_lib_gen: pipeline.out
	HL_DEBUG_CODEGEN=$(CODEGEN_LVL) DYLD_LIBRARY_PATH=$(HALIDE_LIB_PATH) ./pipeline.out > log.txt 2>&1

metrics_gen: metrics.cpp
    g++ -std=c++17 -c metrics.cpp -o metrics.o

driver_gen: driver.cpp metrics.o sca_analysis.a
	g++ -std=c++17  -I $(HALIDE_SRC_PATH)/src/runtime driver.cpp sca_analysis.a metrics.o -o generate_metrics

run_driver: generate_metrics 
	./generate_metrics

clean:
	rm -rf pipeline.out pipeline.dSYM sca_analysis.h sca_analysis.a generate_metrics log.txt metrics.o