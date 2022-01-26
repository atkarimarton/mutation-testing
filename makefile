HOST_GCC= g++
TARGET_GCC= gcc
SOURCE_FILES= program.c
PLUGIN_SOURCE_FILES= plugin.cpp
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS= -I$(GCCPLUGINS_DIR)/include -shared -fPIC -o $@ $^
CFLAGS= -fplugin=./plugin.so -o $@

program: $(SOURCE_FILES) plugin.so
	$(TARGET_GCC) $(CFLAGS) $(SOURCE_FILES)

plugin.so: $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) $(CXXFLAGS)

clean:
	rm -f *.so
	rm -f *.out
	rm -f program
