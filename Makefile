#.PHONY: all clean test

EXEC_FILE = mon_watch

MEM_DEBUG = -fsanitize=address
CFLAGS += -g -Wall -Wextra -Werror -O3 $(MEM_DEBUG)

all: $(EXEC_FILE)

test: $(EXEC_FILE)
	./test.sh

clean:
	rm -f $(EXEC_FILE) *.o
	rm -f *.aux *.log *.out *.pdf
