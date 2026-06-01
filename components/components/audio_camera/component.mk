# COMPONENT_ADD_INCLUDEDIRS := include 

# COMPONENT_SRCDIRS := ./ 

# LIB_FILES := $(shell ls $(COMPONENT_PATH)/lib*.a) \

# LIBS := $(patsubst lib%.a,-l%,$(LIB_FILES))

# COMPONENT_ADD_LDFLAGS +=  -L$(COMPONENT_PATH) \
# 						   $(LIBS)

# COMPONENT_ADD_INCLUDEDIRS := include
# LIBS := canLib
# COMPONENT_ADD_LDFLAGS := $(COMPONENT_PATH)/lib/libcanLib.a
# COMPONENT_ADD_LINKER_DEPS := $(patsubst %,$(COMPONENT_PATH)/lib/lib%.a,$(LIBS))


