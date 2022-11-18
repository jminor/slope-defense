# Makefile as a convenience
# Actual build is done with CMake

all: build_all run
	@echo Build complete for $(EXE)

build_all:
	mkdir -p build
	(cd build; emcmake cmake .. && cmake --build .)

run:
	emrun build/slope_defense.html
	#python3 -m http.server

clean:
	rm -rf build

