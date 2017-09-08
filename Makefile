TEMPDIR := $(shell mktemp -d -u)
ROOTDIR := $(shell pwd)
# VERSION := $(shell git describe --long --tags | sed 's/\([^-]*\)-.*/\1/g')
HEADERS := $(shell find include -name \*.[ch]pp -not -name undef\*)
SRC := $(shell find test -name \*.[ch]pp) $(HEADERS)
ZIP := cool-$(VERSION).zip

all: test

test:
	@$(MAKE) --no-print-directory -C test
	@echo "Running test suite..."
	@valgrind --error-exitcode=1 --leak-check=full test/test_suite -d yes

format:
	@echo Formatting source...
	@clang-format -i -style=file $(SRC)

tidy:
	@echo Tidying source...
	@clang-tidy $(HEADERS) -fix -fix-errors -- -std=c++11 -isystem./include

clean:
	@$(MAKE) --no-print-directory -C test clean
	@echo Cleaning gcov files...
	@find . -name '*.gcno' -exec rm {} \;
	@find . -name '*.gcda' -exec rm {} \;
	@find . -name '*.gcov' -exec rm {} \;

release: $(ZIP)

$(ZIP):
	util/release $(ZIP) $(TEMPDIR) $(ROOTDIR)

.PHONY: format test clean tidy release
