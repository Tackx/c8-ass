build:
	cmake --build build

test: build
	ctest --test-dir build --output-on-failure

run: build
	./build/ass.exe test.ass

.PHONY: build test run