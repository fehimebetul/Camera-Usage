########################################################################
####################### Makefile Template ##############################
########################################################################

# Compiler settings - Can be customized.
CC = g++ -std=c++11
CXXFLAGS = -g -Wall

# Makefile settings - Can be customized.
APPNAME = main
EXT = .cpp
SRCDIR = ./
OBJDIR = ./

############## Do not change anything from here downwards! #############
SRC = $(wildcard $(SRCDIR)/*$(EXT))
OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)
DEP = $(OBJ:$(OBJDIR)/%.o=%.d)
# UNIX-based OS variables & settings
RM = rm
DELOBJ = $(OBJ)
# Windows OS variables & settings
DEL = del
EXE = .exe
WDELOBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)\\%.o)

INCS = -I inc -I /usr/local/include
LIBS = -L/usr/local/lib
LIBS += \
    -lopencv_core \
    -lopencv_videoio \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_imgproc \
    -lopencv_calib3d \
    -ldl \
    -lboost_system \
    -lpthread

PYLON_ROOT ?= /opt/pylon5
CXXFLAGS   += $(shell $(PYLON_ROOT)/bin/pylon-config --cflags)
LIBS    += $(shell $(PYLON_ROOT)/bin/pylon-config --libs-rpath)
LIBS     += $(shell $(PYLON_ROOT)/bin/pylon-config --libs)

ifeq ($(NVCC_TEST),$(NVCC)) 
LIBS += -L/usr/local/cuda-8.0/lib64
INCS += -I ./inc/cuda-8.0 -I /usr/local/cuda-8.0/include   

LIBS += \
    -lopencv_video \
    -lopencv_flann \
    -lopencv_features2d \
    -lopencv_xfeatures2d \
    -lopencv_calib3d \
    -lopencv_cudafilters \
    -lopencv_cudafeatures2d \
    -lopencv_cudawarping \
    -lopencv_cudalegacy \
    -lopencv_cudaimgproc \
    -lopencv_cudaarithm \
    -lopencv_cudaoptflow \

CXXFLAGS += -D WITH_CUDA
endif
########################################################################
####################### Targets beginning here #########################
########################################################################

all: $(APPNAME)

# Builds the app
$(APPNAME): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Creates the dependecy rules
%.d: $(SRCDIR)/%$(EXT)
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:%.d=$(OBJDIR)/%.o) >$@

# Includes all .h files
-include $(DEP)

# Building rule for .o files and its .c/.cpp in combination with all .h
$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT)
	$(CC) $(CXXFLAGS) -o $@ -c $<

################### Cleaning rules for Unix-based OS ###################
# Cleans complete project
.PHONY: clean
clean:
	$(RM) $(DELOBJ) $(DEP) $(APPNAME)

# Cleans only all files with the extension .d
.PHONY: cleandep
cleandep:
	$(RM) $(DEP)
