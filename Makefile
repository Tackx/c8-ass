build:
	cmake --preset gcc-debug && cmake --build --preset gcc-debug

test:
	cmake --workflow --preset gcc-debug

run: build
	./build/gcc-debug/ass.exe test.ass

build-release:
	cmake --workflow --preset gcc-release

run-release: build-release
	./build/gcc-release/ass.exe test.ass

.PHONY: build test run build-release run-release
