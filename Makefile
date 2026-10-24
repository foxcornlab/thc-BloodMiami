CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -pthread
LDFLAGS = -lcurl -pthread
TARGET = bloodmiami
SRCDIR = src
UTILDIR = utils
INCDIR = include
BUILDDIR = build
TESTDIR = tests

MAIN_SRC = bloodmiami.c
CORE_SRCS = $(SRCDIR)/core.c $(SRCDIR)/parser.c $(SRCDIR)/logger.c \
            $(SRCDIR)/inject_prompt.c $(SRCDIR)/inject_lang.c \
            $(SRCDIR)/utils.c
UTIL_SRCS = $(UTILDIR)/http.c
ALL_SRCS = $(MAIN_SRC) $(CORE_SRCS) $(UTIL_SRCS)

OBJS = $(ALL_SRCS:%.c=$(BUILDDIR)/%.o)

all: $(BUILDDIR) $(BUILDDIR)/$(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR)/$(SRCDIR)
	mkdir -p $(BUILDDIR)/$(UTILDIR)

$(BUILDDIR)/$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "BloodMiami compiled successfully!"

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

install: $(BUILDDIR)/$(TARGET)
	cp $(BUILDDIR)/$(TARGET) /usr/local/bin/
	chmod +x /usr/local/bin/$(TARGET)
	@echo "BloodMiami installed to /usr/local/bin/"

clean:
	rm -rf $(BUILDDIR)
	@echo "🧹 Build directory cleaned"

debug: CFLAGS += -g -DDEBUG
debug: clean all

release: CFLAGS += -O3 -DNDEBUG
release: clean all

test: $(BUILDDIR)/$(TARGET)
	@echo "Running tests..."
	cd $(TESTDIR) && ./test_targets.sh

memcheck: debug
	valgrind --leak-check=full --show-leak-kinds=all $(BUILDDIR)/$(TARGET) --help

dist: clean
	tar -czf bloodmiami-$(shell date +%Y%m%d).tar.gz \
		*.c $(SRCDIR)/ $(UTILDIR)/ $(INCDIR)/ $(TESTDIR)/ wordlist/ \
		Makefile README.md LICENSE

help:
	@echo "BloodMiami Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all      - Build BloodMiami (default)"
	@echo "  debug    - Build with debug symbols"
	@echo "  release  - Build optimized release version"
	@echo "  install  - Install to /usr/local/bin"
	@echo "  clean    - Remove build files"
	@echo "  test     - Run test suite"
	@echo "  memcheck - Run with valgrind (requires debug build)"
	@echo "  dist     - Create distribution package"
	@echo "  help     - Show this help message"

.PHONY: all clean debug release install test memcheck dist help