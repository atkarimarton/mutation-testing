HOST_GCC= g++
TARGET_GCC= gcc
SOURCE_FILES= src/program.c
PLUGIN_SOURCE_FILES= plugin/plugin.cpp
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS= -I$(GCCPLUGINS_DIR)/include -shared -fPIC -o $@ $^
CFLAGS= -fplugin=./plugin.so -fplugin-arg-plugin-rule=1 -fplugin-arg-plugin-target_function=func -o $@

all: plugin.so
	time -f "Total time elapsed: %e sec" ./script.sh

plugin.so: $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) $(CXXFLAGS)

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.out
	rm -f -r tmp
	rm -f -r result
	rm -f program
	rm -f testExample
