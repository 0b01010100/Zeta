# Compiler and flags
CC = gcc

# Flags for Debug and Release builds
CFLAGS_DEBUG = -g -Wall -std=c11    # Debug flags: enable debugging symbols and warnings
CFLAGS_RELEASE = -O2 -Wall -std=c11 # Release flags: optimize for speed and include warnings

# Directories
build_dir_debug = build/debug
build_dir_release = build/release
bin_dir = bin

# Source files
src = $(wildcard *.c)

# Object files for Debug and Release builds
obj_debug = $(patsubst %.c,$(build_dir_debug)/%.o,$(src))
obj_release = $(patsubst %.c,$(build_dir_release)/%.o,$(src))

# Output executables
output_debug = $(bin_dir)/zeta_debug
output_release = $(bin_dir)/zeta

# Default target
all: debug release

# Debug build target
debug: $(output_debug)

$(output_debug): $(obj_debug) | $(bin_dir)
	$(CC) $(obj_debug) -o $(output_debug)

$(build_dir_debug)/%.o: %.c | $(build_dir_debug)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

$(build_dir_debug):
	mkdir -p $(build_dir_debug)

# Release build target
release: $(output_release)

$(output_release): $(obj_release) | $(bin_dir)
	$(CC) $(obj_release) -o $(output_release)

$(build_dir_release)/%.o: %.c | $(build_dir_release)
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@

$(build_dir_release):
	mkdir -p $(build_dir_release)

# Ensure bin directory exists
$(bin_dir):
	mkdir -p $(bin_dir)

# Run the debug build
run-debug: debug
	./$(output_debug)

# Run the release build
run-release: release
	./$(output_release)

# Clean target to remove compiled files and directories
clean:
	rm -rf $(build_dir_debug) $(build_dir_release) $(bin_dir)