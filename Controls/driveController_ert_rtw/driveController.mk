###########################################################################
## Makefile generated for component 'driveController'. 
## 
## Makefile     : driveController.mk
## Generated on : Fri Jul 31 11:03:24 2026
## Final product: $(RELATIVE_PATH_TO_ANCHOR)/driveController.elf
## Product type : executable
## 
###########################################################################

###########################################################################
## MACROS
###########################################################################

# Macro Descriptions:
# PRODUCT_NAME            Name of the system to build
# MAKEFILE                Name of this makefile
# COMPILER_COMMAND_FILE   Compiler command listing model reference header paths
# CMD_FILE                Command file

PRODUCT_NAME              = driveController
MAKEFILE                  = driveController.mk
MATLAB_ROOT               = Z:/.Programmi/MATLAB26a
MATLAB_BIN                = Z:/.Programmi/MATLAB26a/bin
MATLAB_ARCH_BIN           = $(MATLAB_BIN)/win64
START_DIR                 = E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/Controls
SOLVER                    = 
SOLVER_OBJ                = 
CLASSIC_INTERFACE         = 0
TGT_FCN_LIB               = ISO_C
MODEL_HAS_DYNAMICALLY_LOADED_SFCNS = 0
RELATIVE_PATH_TO_ANCHOR   = ..
COMPILER_COMMAND_FILE     = driveController_comp.rsp
CMD_FILE                  = driveController.rsp
C_STANDARD_OPTS           = 
CPP_STANDARD_OPTS         = 

###########################################################################
## TOOLCHAIN SPECIFICATIONS
###########################################################################

# Toolchain Name:          GNU Tools for STM32
# Supported Version(s):    
# ToolchainInfo Version:   2026a
# Specification Revision:  1.0
# 
#-------------------------------------------
# Macros assumed to be defined elsewhere
#-------------------------------------------

# TARGET_LOAD_CMD_ARGS
# TARGET_LOAD_CMD
# MW_GNU_ARM_STM32_PATH
# FDATASECTIONS_FLG

#-----------
# MACROS
#-----------

LIBGCC                    = ${shell $(MW_GNU_ARM_STM32_PATH)/arm-none-eabi-gcc ${CFLAGS} -print-libgcc-file-name}
LIBC                      = ${shell $(MW_GNU_ARM_STM32_PATH)/arm-none-eabi-gcc ${CFLAGS} -print-file-name=libc.a}
LIBM                      = ${shell $(MW_GNU_ARM_STM32_PATH)/arm-none-eabi-gcc ${CFLAGS} -print-file-name=libm.a}
PRODUCT_NAME_WITHOUT_EXTN = $(basename $(PRODUCT))
PRODUCT_BIN               = $(PRODUCT_NAME_WITHOUT_EXTN).bin
PRODUCT_HEX               = $(PRODUCT_NAME_WITHOUT_EXTN).hex
CPFLAGS                   = -O binary
SHELL                     = %SystemRoot%/system32/cmd.exe

TOOLCHAIN_SRCS = 
TOOLCHAIN_INCS = 
TOOLCHAIN_LIBS = -lm

FORMAT_FOR_ECHO_CMD              = $(strip $(subst >,^>,\
	$(subst <,^<,\
	$(subst |,^|,\
	$(subst &,^&,\
	$(subst ",^",\
	$(subst ^,^^,\
	$1)))))))
FORMAT_FOR_ECHO                  = $(FORMAT_FOR_ECHO_CMD)
HASH                             = \#
SEMICOLON                        = ;
UNESCAPE_SEMICOLONS              = $(subst \;,;,$1)
ADD_QUOTES                       = $(foreach aPath,$1,"$(aPath)")

#------------------------
# BUILD TOOL COMMANDS
#------------------------

# Assembler: GNU ARM Assembler
AS_PATH = $(MW_GNU_ARM_STM32_PATH)
AS = "$(AS_PATH)/arm-none-eabi-gcc"

# C Compiler: GNU ARM C Compiler
CC_PATH = $(MW_GNU_ARM_STM32_PATH)
CC = "$(CC_PATH)/arm-none-eabi-gcc"

# Linker: GNU ARM Linker
LD_PATH = $(MW_GNU_ARM_STM32_PATH)
LD = "$(LD_PATH)/arm-none-eabi-g++"

# C++ Compiler: GNU ARM C++ Compiler
CPP_PATH = $(MW_GNU_ARM_STM32_PATH)
CPP = "$(CPP_PATH)/arm-none-eabi-g++"

# C++ Linker: GNU ARM C++ Linker
CPP_LD_PATH = $(MW_GNU_ARM_STM32_PATH)
CPP_LD = "$(CPP_LD_PATH)/arm-none-eabi-g++"

# Archiver: GNU ARM Archiver
AR_PATH = $(MW_GNU_ARM_STM32_PATH)
AR = "$(AR_PATH)/arm-none-eabi-ar"

# MEX Tool: MEX Tool
MEX_PATH = $(MATLAB_ARCH_BIN)
MEX = "$(MEX_PATH)/mex"

# Binary Converter: Binary Converter
OBJCOPYPATH = $(MW_GNU_ARM_STM32_PATH)
OBJCOPY = "$(OBJCOPYPATH)/arm-none-eabi-objcopy"

# Hex Converter: Hex Converter
OBJCOPYPATH = $(MW_GNU_ARM_STM32_PATH)
OBJCOPY = "$(OBJCOPYPATH)/arm-none-eabi-objcopy"

# Download: Download
DOWNLOAD =

# Execute: Execute
EXECUTE = $(PRODUCT)

# Builder: GMAKE Utility
MAKE_PATH = %MATLAB%\bin\win64
MAKE = "$(MAKE_PATH)/gmake"


#-------------------------
# Directives/Utilities
#-------------------------

ASDEBUG             = -g
AS_OUTPUT_FLAG      = -o
CDEBUG              = -g
C_OUTPUT_FLAG       = -o
LDDEBUG             = -g
OUTPUT_FLAG         = -o
CPPDEBUG            = -g
CPP_OUTPUT_FLAG     = -o
CPPLDDEBUG          = -g
OUTPUT_FLAG         = -o
ARDEBUG             =
STATICLIB_OUTPUT_FLAG =
MEX_DEBUG           = -g
RM                  = @del /f/q
ECHO                = @echo
MV                  = @move
RUN                 =

#----------------------------------------
# "Faster Builds" Build Configuration
#----------------------------------------

ARFLAGS              = ruvs
ASFLAGS              = -MMD -MP -MF"$(@:%.s.o=%.s.dep)" -MT"$@"  \
                       -Wall \
                       -x assembler-with-cpp \
                       $(ASFLAGS_ADDITIONAL) \
                       $(DEFINES) \
                       $(INCLUDES) \
                       -c
OBJCOPYFLAGS_BIN     = -O binary $(PRODUCT) $(PRODUCT_BIN)
CFLAGS               = $(FDATASECTIONS_FLG) \
                       -Wall \
                       -c \
                       -MMD -MP -MF"$(@:%.c.o=%.c.dep)" -MT"$@"  \
                       -O0
CPPFLAGS             = -std=gnu++14 \
                       -fno-rtti \
                       -fno-exceptions \
                       $(FDATASECTIONS_FLG) \
                       -Wall \
                       -c \
                       -MMD -MP -MF"$(@:%.cpp.o=%.cpp.dep)" -MT"$@"  \
                       -O0
CPP_LDFLAGS          = -Wl,--gc-sections \
                       -Wl,-Map="$(PRODUCT_NAME).map" \
                       -Wl,--print-memory-usage
CPP_SHAREDLIB_LDFLAGS  =
DOWNLOAD_FLAGS       =
EXECUTE_FLAGS        =
OBJCOPYFLAGS_HEX     = -O ihex $(PRODUCT) $(PRODUCT_HEX)
LDFLAGS              = -Wl,--gc-sections \
                       -Wl,-Map="$(PRODUCT_NAME).map" \
                       -Wl,--print-memory-usage
MEX_CPPFLAGS         =
MEX_CPPLDFLAGS       =
MEX_CFLAGS           =
MEX_LDFLAGS          =
MAKE_FLAGS           = -f $(MAKEFILE)
SHAREDLIB_LDFLAGS    =



###########################################################################
## OUTPUT INFO
###########################################################################

PRODUCT = $(RELATIVE_PATH_TO_ANCHOR)/driveController.elf
PRODUCT_TYPE = "executable"
BUILD_TYPE = "Top-Level Standalone Executable"

###########################################################################
## INCLUDE PATHS
###########################################################################

INCLUDES_BUILDINFO = 

INCLUDES = $(INCLUDES_BUILDINFO)

###########################################################################
## DEFINES
###########################################################################

DEFINES_BUILD_ARGS = -DCLASSIC_INTERFACE=0 -DALLOCATIONFCN=0 -DTERMFCN=1 -DONESTEPFCN=1 -DMAT_FILE=0 -DMULTI_INSTANCE_CODE=0 -DINTEGER_CODE=0 -DMT=0
DEFINES_CUSTOM = 
DEFINES_OPTS = -DTID01EQ=0
DEFINES_STANDARD = -DMODEL=driveController -DNUMST=3 -DNCSTATES=0 -DHAVESTDIO -DMODEL_HAS_DYNAMICALLY_LOADED_SFCNS=0

DEFINES = $(DEFINES_BUILD_ARGS) $(DEFINES_CUSTOM) $(DEFINES_OPTS) $(DEFINES_STANDARD)

###########################################################################
## SOURCE FILES
###########################################################################

SRCS = $(START_DIR)/driveController_ert_rtw/driveController.c $(START_DIR)/driveController_ert_rtw/driveController_data.c $(START_DIR)/driveController_ert_rtw/rtGetNaN.c $(START_DIR)/driveController_ert_rtw/rt_nonfinite.c

MAIN_SRC = $(START_DIR)/driveController_ert_rtw/ert_main.c

ALL_SRCS = $(SRCS) $(MAIN_SRC)

###########################################################################
## OBJECTS
###########################################################################

OBJS = driveController.c.o driveController_data.c.o rtGetNaN.c.o rt_nonfinite.c.o

MAIN_OBJ = ert_main.c.o

ALL_OBJS = $(OBJS) $(MAIN_OBJ)

###########################################################################
## PREBUILT OBJECT FILES
###########################################################################

PREBUILT_OBJS = 

###########################################################################
## LIBRARIES
###########################################################################

LIBS = 

###########################################################################
## SYSTEM LIBRARIES
###########################################################################

SYSTEM_LIBS = 

###########################################################################
## ADDITIONAL TOOLCHAIN FLAGS
###########################################################################

#---------------
# C Compiler
#---------------

CFLAGS_BASIC = $(DEFINES) $(INCLUDES) @$(COMPILER_COMMAND_FILE)

CFLAGS += $(CFLAGS_BASIC)

#-----------------
# C++ Compiler
#-----------------

CPPFLAGS_BASIC = $(DEFINES) $(INCLUDES) @$(COMPILER_COMMAND_FILE)

CPPFLAGS += $(CPPFLAGS_BASIC)

#---------------------
# MEX C++ Compiler
#---------------------

MEX_CPP_Compiler_BASIC =  @$(COMPILER_COMMAND_FILE)

MEX_CPPFLAGS += $(MEX_CPP_Compiler_BASIC)

#-----------------
# MEX Compiler
#-----------------

MEX_Compiler_BASIC =  @$(COMPILER_COMMAND_FILE)

MEX_CFLAGS += $(MEX_Compiler_BASIC)

###########################################################################
## INLINED COMMANDS
###########################################################################


ALL_DEPS:=$(patsubst %.o,%.dep,$(ALL_OBJS))
all:

ifndef DISABLE_GCC_FUNCTION_DATA_SECTIONS
FDATASECTIONS_FLG := -ffunction-sections -fdata-sections
endif



-include codertarget_assembly_flags.mk
-include ../codertarget_assembly_flags.mk
-include ../../codertarget_assembly_flags.mk
-include mw_gnu_tools_for_stm32_path.mk
-include ../mw_gnu_tools_for_stm32_path.mk
-include ../../mw_gnu_tools_for_stm32_path.mk
-include $(ALL_DEPS)


###########################################################################
## PHONY TARGETS
###########################################################################

.PHONY : all build buildobj clean info prebuild postbuild download execute


all : build postbuild
	@echo $(call FORMAT_FOR_ECHO,### Successfully generated all binary outputs.)


build : prebuild $(PRODUCT)


buildobj : prebuild $(OBJS) $(PREBUILT_OBJS)
	@echo $(call FORMAT_FOR_ECHO,### Successfully generated all binary outputs.)


prebuild : 


postbuild : $(PRODUCT)
	@echo $(call FORMAT_FOR_ECHO,### Invoking postbuild tool Binary Converter ...)
	$(OBJCOPY) $(OBJCOPYFLAGS_BIN)
	@echo $(call FORMAT_FOR_ECHO,### Done invoking postbuild tool.)
	@echo $(call FORMAT_FOR_ECHO,### Invoking postbuild tool Hex Converter ...)
	$(OBJCOPY) $(OBJCOPYFLAGS_HEX)
	@echo $(call FORMAT_FOR_ECHO,### Done invoking postbuild tool.)


download : postbuild


execute : download
	@echo $(call FORMAT_FOR_ECHO,### Invoking postbuild tool Execute ...)
	$(EXECUTE) $(EXECUTE_FLAGS)
	@echo $(call FORMAT_FOR_ECHO,### Done invoking postbuild tool.)


###########################################################################
## FINAL TARGET
###########################################################################

#-------------------------------------------
# Create a standalone executable            
#-------------------------------------------

$(PRODUCT) : $(OBJS) $(PREBUILT_OBJS) $(MAIN_OBJ)
	@echo $(call FORMAT_FOR_ECHO,### Creating standalone executable "$(PRODUCT)" ...)
	$(LD) $(LDFLAGS) -o $(PRODUCT) @$(CMD_FILE) $(SYSTEM_LIBS) $(TOOLCHAIN_LIBS)
	@echo $(call FORMAT_FOR_ECHO,### Created: "$(PRODUCT)")


###########################################################################
## INTERMEDIATE TARGETS
###########################################################################

#---------------------
# SOURCE-TO-OBJECT
#---------------------

%.c.o : %.c
	$(CC) $(CFLAGS) -o "$@" "$<"


%.cpp.o : %.cpp
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : %.cc
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : %.C
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : %.cxx
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.s.o : %.s
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.s.o : %.S
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.c.o : $(RELATIVE_PATH_TO_ANCHOR)/%.c
	$(CC) $(CFLAGS) -o "$@" "$<"


%.cpp.o : $(RELATIVE_PATH_TO_ANCHOR)/%.cpp
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(RELATIVE_PATH_TO_ANCHOR)/%.cc
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(RELATIVE_PATH_TO_ANCHOR)/%.C
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(RELATIVE_PATH_TO_ANCHOR)/%.cxx
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.s.o : $(RELATIVE_PATH_TO_ANCHOR)/%.s
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.s.o : $(RELATIVE_PATH_TO_ANCHOR)/%.S
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.c.o : $(START_DIR)/%.c
	$(CC) $(CFLAGS) -o "$@" "$<"


%.cpp.o : $(START_DIR)/%.cpp
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(START_DIR)/%.cc
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(START_DIR)/%.C
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(START_DIR)/%.cxx
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.s.o : $(START_DIR)/%.s
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.s.o : $(START_DIR)/%.S
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.c.o : $(START_DIR)/driveController_ert_rtw/%.c
	$(CC) $(CFLAGS) -o "$@" "$<"


%.cpp.o : $(START_DIR)/driveController_ert_rtw/%.cpp
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(START_DIR)/driveController_ert_rtw/%.cc
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(START_DIR)/driveController_ert_rtw/%.C
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(START_DIR)/driveController_ert_rtw/%.cxx
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.s.o : $(START_DIR)/driveController_ert_rtw/%.s
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.s.o : $(START_DIR)/driveController_ert_rtw/%.S
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.c.o : $(MATLAB_ROOT)/rtw/c/src/%.c
	$(CC) $(CFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/rtw/c/src/%.cpp
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/rtw/c/src/%.cc
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/rtw/c/src/%.C
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/rtw/c/src/%.cxx
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.s.o : $(MATLAB_ROOT)/rtw/c/src/%.s
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.s.o : $(MATLAB_ROOT)/rtw/c/src/%.S
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.c.o : $(MATLAB_ROOT)/simulink/src/%.c
	$(CC) $(CFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/simulink/src/%.cpp
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/simulink/src/%.cc
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/simulink/src/%.C
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/simulink/src/%.cxx
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.s.o : $(MATLAB_ROOT)/simulink/src/%.s
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.s.o : $(MATLAB_ROOT)/simulink/src/%.S
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.c.o : $(MATLAB_ROOT)/toolbox/simulink/blocks/src/%.c
	$(CC) $(CFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/toolbox/simulink/blocks/src/%.cpp
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/toolbox/simulink/blocks/src/%.cc
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/toolbox/simulink/blocks/src/%.C
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.cpp.o : $(MATLAB_ROOT)/toolbox/simulink/blocks/src/%.cxx
	$(CPP) $(CPPFLAGS) -o "$@" "$<"


%.s.o : $(MATLAB_ROOT)/toolbox/simulink/blocks/src/%.s
	$(AS) $(ASFLAGS) -o "$@" "$<"


%.s.o : $(MATLAB_ROOT)/toolbox/simulink/blocks/src/%.S
	$(AS) $(ASFLAGS) -o "$@" "$<"


driveController.c.o : $(START_DIR)/driveController_ert_rtw/driveController.c
	$(CC) $(CFLAGS) -o "$@" "$<"


driveController_data.c.o : $(START_DIR)/driveController_ert_rtw/driveController_data.c
	$(CC) $(CFLAGS) -o "$@" "$<"


ert_main.c.o : $(START_DIR)/driveController_ert_rtw/ert_main.c
	$(CC) $(CFLAGS) -o "$@" "$<"


rtGetNaN.c.o : $(START_DIR)/driveController_ert_rtw/rtGetNaN.c
	$(CC) $(CFLAGS) -o "$@" "$<"


rt_nonfinite.c.o : $(START_DIR)/driveController_ert_rtw/rt_nonfinite.c
	$(CC) $(CFLAGS) -o "$@" "$<"


###########################################################################
## DEPENDENCIES
###########################################################################

$(ALL_OBJS) : rtw_proj.tmw $(COMPILER_COMMAND_FILE) $(MAKEFILE)


###########################################################################
## MISCELLANEOUS TARGETS
###########################################################################

info : 
	@echo $(call FORMAT_FOR_ECHO,### PRODUCT = $(PRODUCT))
	@echo $(call FORMAT_FOR_ECHO,### PRODUCT_TYPE = $(PRODUCT_TYPE))
	@echo $(call FORMAT_FOR_ECHO,### BUILD_TYPE = $(BUILD_TYPE))
	@echo $(call FORMAT_FOR_ECHO,### INCLUDES = $(INCLUDES))
	@echo $(call FORMAT_FOR_ECHO,### DEFINES = $(DEFINES))
	@echo $(call FORMAT_FOR_ECHO,### ALL_SRCS = $(ALL_SRCS))
	@echo $(call FORMAT_FOR_ECHO,### ALL_OBJS = $(ALL_OBJS))
	@echo $(call FORMAT_FOR_ECHO,### LIBS = $(LIBS))
	@echo $(call FORMAT_FOR_ECHO,### MODELREF_LIBS = $(MODELREF_LIBS))
	@echo $(call FORMAT_FOR_ECHO,### SYSTEM_LIBS = $(SYSTEM_LIBS))
	@echo $(call FORMAT_FOR_ECHO,### TOOLCHAIN_LIBS = $(TOOLCHAIN_LIBS))
	@echo $(call FORMAT_FOR_ECHO,### ASFLAGS = $(ASFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### CFLAGS = $(CFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### LDFLAGS = $(LDFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### SHAREDLIB_LDFLAGS = $(SHAREDLIB_LDFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### CPPFLAGS = $(CPPFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### CPP_LDFLAGS = $(CPP_LDFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### CPP_SHAREDLIB_LDFLAGS = $(CPP_SHAREDLIB_LDFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### ARFLAGS = $(ARFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### MEX_CFLAGS = $(MEX_CFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### MEX_CPPFLAGS = $(MEX_CPPFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### MEX_LDFLAGS = $(MEX_LDFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### MEX_CPPLDFLAGS = $(MEX_CPPLDFLAGS))
	@echo $(call FORMAT_FOR_ECHO,### OBJCOPYFLAGS_BIN = $(OBJCOPYFLAGS_BIN))
	@echo $(call FORMAT_FOR_ECHO,### OBJCOPYFLAGS_HEX = $(OBJCOPYFLAGS_HEX))
	@echo $(call FORMAT_FOR_ECHO,### DOWNLOAD_FLAGS = $(DOWNLOAD_FLAGS))
	@echo $(call FORMAT_FOR_ECHO,### EXECUTE_FLAGS = $(EXECUTE_FLAGS))
	@echo $(call FORMAT_FOR_ECHO,### MAKE_FLAGS = $(MAKE_FLAGS))


clean : 
	$(ECHO) "### Deleting all derived files ..."
	$(RM) $(subst /,\,$(PRODUCT))
	$(RM) $(subst /,\,$(ALL_OBJS))
	$(RM) *.c.dep
	$(RM) *.cpp.dep
	$(RM) *.s.dep
	$(ECHO) "### Deleted all derived files."


