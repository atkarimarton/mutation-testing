HOST_GCC= g++
TARGET_GCC= gcc
SOURCE_FILES= src/program.c
PLUGIN_SOURCE_FILES= plugin/plugin.cpp
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS= -I$(GCCPLUGINS_DIR)/include -shared -fPIC -o $@ $^
REPORT_FLAGS= -c -fplugin=./plugin.so -fplugin-arg-plugin-generate_html_report -fplugin-arg-plugin-resultdir=result

plugin.so: $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) $(CXXFLAGS)

unity_test: plugin.so
	time -f "Total time elapsed: %e sec" ./unity_test.sh

generate_mutants: plugin.so
	time -f "Total time elapsed generating mutants: %e sec" ./generate_mutants.sh

generate_report: plugin.so
	$(TARGET_GCC) $(SOURCE_FILES) $(REPORT_FLAGS) && rm program.o

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.out
	rm -f *.html
	rm -f -r tmp
	rm -f -r result
	rm -f program
	rm -f testExample
