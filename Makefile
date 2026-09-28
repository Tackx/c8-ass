MODE ?= debug
ifneq ($(filter release,$(MAKECMDGOALS)),)
MODE := release
endif

PRESET := gcc-$(MODE)

build:
	cmake --preset $(PRESET) && cmake --build --preset $(PRESET)

run: build
	./build/$(PRESET)/ass.exe test.ass

test:
	cmake --workflow --preset gcc-debug

release:
	@:

.PHONY: build run test release