NAME := fusefat32

SRC_DIR := src
INCLUDE_DIR := include
VPATH := $(SRC_DIR) $(INCLUDE_DIR)
LIBS :=
FUSE_PARAMS := $(shell pkg-config fuse --cflags --libs)

override CFLAGS += --std=c99 -Wall -Werror
override CPPFLAGS += -I $(INCLUDE_DIR)

SOURCES := $(shell find $(SRC_DIR) -name *.c -printf "%f\n")
HEADERS := $(shell find $(INCLUDE_DIR) -name *.h -printf "%f\n")

STANDALONE_TARGETS := clean TAGS
NONSTANDALONE_GOALS := $(strip $(filter-out $(STANDALONE_TARGETS),$(MAKECMDGOALS)))

all: $(NAME)

$(NAME): $(subst .c,.o,$(SOURCES))
	$(CC) $(LD_FLAGS) $(CFLAGS) $(CPPFLAGS) $(FUSE_PARAMS) $^ $(LIBS) -o $@

%.d: %.c
	@$(CC) -M $(CPPFLAGS) $< > $@.$$$$;                 \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.PHONY : clean
clean:
	@-rm -f *.o *.d TAGS ${NAME}

TAGS: $(SOURCES) $(HEADERS)
	@etags $(abspath $^)

ifneq ($(NONSTANDALONE_GOALS),)
include $(subst .c,.d,$(SOURCES))
endif
