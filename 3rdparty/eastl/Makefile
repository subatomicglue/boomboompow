
PROJECT_NAME = eastl


SRCS= allocator.cpp      fixed_pool.cpp     red_black_tree.cpp \
      assert.cpp         hashtable.cpp      string.cpp

OBJDIR = .
DEPDIR = .

PATH_TO_SRCS = .:src
MK_DIR = mk
INCLUDE_PATHS =  -Iinclude -std=c++0x

########################################################################
# system dependent stuff, that users will want to change...
# stuff that cant be abstracted in the build system...
##########################################################################
include ${MK_DIR}/mk.platform
include ${MK_DIR}/mk.hosttype
# IRIX
ifeq ($(HOSTTYPE), irix)
#   LIBS = -lBox2D
   include ${MK_DIR}/mk.irix
endif

#LiNUX
ifeq ($(HOSTTYPE), linux)
#   LIBS = -lBox2D
   include ${MK_DIR}/mk.gnu
endif

# Win32
ifeq ($(HOSTTYPE), Win32)
#   LIBS = Box2D.lib
   DEPENDFLAGS = -D__cplusplus -D_WIN32 -DWIN32 $(INCLUDE_PATHS)
   include ${MK_DIR}/mk.win32
   # Normal definitions
endif

########################################################################

include ${MK_DIR}/mk.objs

default: lib$(PROJECT_NAME).a

# Compile then Link
lib$(PROJECT_NAME).a: $(OBJS)
	-@echo ""
	-@echo "-------------------------------------------"
	-@echo "Linking... (.$(OBJ_FILE_SUFFIX)'s --> .so)"
	-@echo "-------------------------------------------"
#	$(SO_LINKER) $(SO_FLAGS) $(OBJS) $(OUTPUT_LIB_FLAG) $(LIB_FILENAME) $(LIBS)
	$(STATIC_LINKER) $(STATIC_FLAGS) $(OUTPUT_STATIC_LIB_FLAG)$(STATIC_LIB_FILENAME) $(OBJS)
	-@echo "-------------------------------------------"
	-@echo "$(FILENAME) done"
	-@echo "-------------------------------------------"

# Compile then Link
lib$(PROJECT_NAME).so: $(OBJS)
	-@echo ""
	-@echo "-------------------------------------------"
	-@echo "Linking... (.$(OBJ_FILE_SUFFIX)'s --> .so)"
	-@echo "-------------------------------------------"
	$(SO_LINKER) $(SO_FLAGS) $(OBJS) $(OUTPUT_LIB_FLAG) $(LIB_FILENAME) $(LIBS)
	-@echo "-------------------------------------------"
	-@echo "$(FILENAME) done"
	-@echo "-------------------------------------------"

prefix ?= installed


# Remove the compiled stuff from the system
clean:
	-@echo ""
	-@echo "-------------------------------------------"
	-@echo "Removing compiled stuff from the system	 "
	-@echo "-------------------------------------------"
	-rm -r $(OBJS) \
	$(LIB_FILENAME) $(STATIC_LIB_FILENAME) \
	 *.ncb *.opt *.plg *.pch *.idb *.pdb *.ilk Debug/ Release/ ii_files/


clobber: clean
	-rm *.d

-include ${MK_DIR}/mk.depend

