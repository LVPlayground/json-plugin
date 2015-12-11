# Copyright 2012 Las Venturas Playground. All rights reserved.
# Use of this source code is governed by the GPLv2 license, a copy of which can
# be found in the LICENSE file.

# Compile the JSON project on Linux systems. Just run "make"

$(shell [ -d "out" ] || mkdir -p out)

OUTFILE=json-plugin.so
COMPILE_FLAGS=-c -m32 -fPIC -O3 -w -DLINUX -DHAVE_STDINT_H -I.

all:
	g++ $(COMPILE_FLAGS) sdk/amxplugin.cpp -o out/amxplugin.o
	g++ $(COMPILE_FLAGS) block_allocator.cpp -o out/block_allocator.o
	g++ $(COMPILE_FLAGS) json.cpp -o out/json.o
	g++ $(COMPILE_FLAGS) plugin.cpp -o out/plugin.o
	g++ -O2 -m32 -fshort-wchar -Wl,--export-dynamic -shared -o out/$(OUTFILE) out/*.o
