MAIN_FILE = fuselogger.c
MAIN_OUTPUT = fuselogger

FUSE_DIR = dist

build:
	@gcc -Wall $(MAIN_FILE) $(shell pkg-config fuse --cflags --libs) -o $(MAIN_OUTPUT)

unmount:
	@if mountpoint -q $(FUSE_DIR) 2>/dev/null; then \
		fusermount -u $(FUSE_DIR); \
	fi
	
run: build unmount
	@if [ ! -d "$(FUSE_DIR)" ]; then mkdir $(FUSE_DIR); fi
	@./$(MAIN_OUTPUT) $(FUSE_DIR)
