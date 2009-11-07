NAME := fusefat32

SRC_DIR        := src
INCLUDE_DIR    := include

SRC_DIRS       := $(shell find $(SRC_DIR) -type d -printf "%p\n")
INCLUDE_DIRS   := $(shell find $(INCLUDE_DIR) -type d -printf "%p\n")

DOC_DIR        := doc

DOXYGEN_DIR    := $(DOC_DIR)/doxygen
DOXYGEN_CONFIG := $(DOC_DIR)/Doxyfile

VPATH          := $(SRC_DIRS) $(INCLUDE_DIRS) $(DOC_DIR)
FUSE_CFLAGS    := $(shell pkg-config fuse --cflags-only-other)
FUSE_CPPFLAGS  := $(shell pkg-config fuse --cflags-only-I)
FUSE_LIBS      := $(shell pkg-config fuse --libs)

CFLAGS   := --std=c99 -O2 -Wall -Winline $(FUSE_CFLAGS)
CPPFLAGS := -iquote$(INCLUDE_DIR) $(FUSE_CPPFLAGS)
LIBS     := $(FUSE_LIBS)

SOURCES := $(shell find $(SRC_DIR) -name *.c -printf "%f\n")
HEADERS := $(shell find $(INCLUDE_DIR) -name *.h -printf "%f\n")

STANDALONE_TARGETS := clean TAGS doxygen doxygen-clean
NONSTANDALONE_GOALS := \
    $(strip $(filter-out $(STANDALONE_TARGETS),$(MAKECMDGOALS)))

.PHONY : all
all: $(NAME)

$(NAME): $(subst .c,.o,$(SOURCES))
	$(CC) $(LD_FLAGS) $(CFLAGS) $(CPPFLAGS) $^ $(LIBS) -o $@

doxygen: $(DOXYGEN_CONFIG) $(SOURCES) $(HEADERS)
	@doxygen $(DOXYGEN_CONFIG)

.PHONY : doxygen-clean
doxygen-clean:
	@-rm -rf $(DOXYGEN_DIR)


%.d: %.c
	@$(CC) -M $(CPPFLAGS) $< > $@.$$$$;                 \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.PHONY : clean
clean: doxygen-clean
	@-rm -f *.o *.d TAGS ${NAME}

TAGS: $(SOURCES) $(HEADERS)
	@etags $(abspath $^)

ifneq ($(NONSTANDALONE_GOALS),)
include $(subst .c,.d,$(SOURCES))
endif
