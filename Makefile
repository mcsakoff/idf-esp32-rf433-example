PROJECT_NAME := firmware

.PHONY: open help help-idf build clean flash monitor reconfigure size clobber
default: help

END = \x1b[0m
RED = \x1b[31;01m
GRN = \x1b[32;01m
YEL = \x1b[33;01m

help: Makefile
	@echo "$(GRN)Choose a command run:$(END)"
	@sed -n 's/^## //p' $< | column -t -s ':' | sed -e 's/^/    /'
	@echo

# commented to disable default make build (use idf.py and CMake instead)
# include $(IDF_PATH)/make/project.mk

## open: Open the project in CLion
open:
	@if [ -z "$$IDF_PATH" ]; then \
		echo "---> Setting up ESP-IDF SDK environment"; \
		source .envrc; \
	fi
	@echo "---> Running CLion"
	@/usr/local/bin/clion .

IDF = ${IDF_PATH}/tools/idf.py

## help-idf: print IDF help
help-idf:
	@${IDF} --help

## menuconfig: Run "menuconfig" project configuration tool.
menuconfig:
	@${IDF} menuconfig

## build: Build the project
build:
	@${IDF} build

## clean: Delete build output files from the build directory.
clean:
	@${IDF} clean

## flash: Flash the project.
flash:
	@${IDF} flash

## monitor: Display serial output.
monitor:
	@${IDF} monitor

## reconfigure: Re-run CMake.
reconfigure:
	@${IDF} reconfigure

## size: Print basic size information about the app.
size:
	@${IDF} size

## clobber: Delete the entire build directory contents.
clobber:
	@${IDF} fullclean
	@rm -f sdkconfig
	@rm -f sdkconfig.old
