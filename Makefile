MODE ?= debug
ifneq ($(filter release,$(MAKECMDGOALS)),)
MODE := release
endif

PRESET := gcc-$(MODE)

all: build test run

build:
	cmake --preset $(PRESET) && cmake --build --preset $(PRESET)

run: build
	./build/$(PRESET)/ass.exe test.ass

test:
	cmake --workflow --preset gcc-debug

release: all

.PHONY: build run test release