build:
	cmake -B build/debug -G Ninja -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe -DCMAKE_BUILD_TYPE=Debug && cmake --build build/debug --parallel

test: build
	ctest --test-dir build/debug --output-on-failure

run: build
	./build/debug/ass.exe test.ass

build-release:
	cmake -B build/release -G Ninja -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF && cmake --build build/release --parallel

run-release: build-release
	./build/release/ass.exe test.ass

.PHONY: build test run