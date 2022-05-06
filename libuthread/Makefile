# Target library
lib    := libuthread.a
objs   := uthread.o sem.o queue.o preempt.o context.o

# GCC parameter
CC     := gcc
CFLAGS := -Wall -Wextra -Werror -MMD

# Compilation Optimization
CFLAGS += -O2

# Target
all: $(lib)

# Dep tracking *must* be below the 'all' rule
deps := $(patsubst %.o,%.d,$(objs))
-include deps

# Rule for compiling the library
libuthread.a: $(objs)
	ar rcs $@ $^

# Rule for compiling objs
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule for removing all generated files
clean:
	rm -f $(lib) $(objs) $(deps)