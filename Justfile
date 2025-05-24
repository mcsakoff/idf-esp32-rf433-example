IDF := "idf.py"

# List available recipes
usage:
    @just --list --unsorted --list-prefix "  " --justfile "{{justfile()}}"

# Get devkit board info
info:
    espflash board-info --no-stub

# Set target
set-target TARGET:
    "{{IDF}}" set-target "{{TARGET}}"

# Open the project in CLion
open:
    @/Applications/CLion.app/Contents/MacOS/clion . > /dev/null 2>&1 &

# Print IDF help
help-idf:
    "{{IDF}}" --help

# Run "menuconfig" project configuration tool
menuconfig:
    "{{IDF}}" menuconfig

# Build the project
build:
    "{{IDF}}" all

# Delete build output files from the build directory
clean:
    "{{IDF}}" clean

# Flash the project
flash:
    "{{IDF}}" flash

# Display serial output.
monitor:
    "{{IDF}}" monitor

# Re-run CMake
reconfigure:
    "{{IDF}}" reconfigure

# Build, Flash and Monitor
bfm: build flash monitor

# Print basic size information about the app
size:
    "{{IDF}}" size

# Clean up the entire project's directory contents
clobber:
    "{{IDF}}" fullclean
    @rm -f sdkconfig
    @rm -f sdkconfig.old
    @rm -rf cmake-build-*
    @rm -rf build
    @rm -rf cache
