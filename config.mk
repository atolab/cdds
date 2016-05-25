ifeq "$(CONFIG)" ""
  override CONFIG := $(shell ./guess-config)
  ifeq "$(CONFIG)" ""
    $(error "Failed to guess config")
  endif
endif

ifeq "$(VDDS_HOME)" ""
  override VDDS_HOME := $(PWD)
endif

OS := $(shell echo $(CONFIG) | sed -e 's/^[^.]*\.//' -e 's/^\([^-_]*\)[-_].*/\1/')
PROC := $(shell echo $(CONFIG) | sed -e 's/^\([^.]*\)\..*/\1/')

ifeq "$(OS)" "darwin"
  CC = clang
  LD = $(CC)
  OPT = -fsanitize=address #-O3 -DNDEBUG
  PROF =
  CPPFLAGS += -Wall -g $(OPT) $(PROF)
  CFLAGS += $(CPPFLAGS) #-fno-inline
  LDFLAGS += -g $(OPT) $(PROF)
  X =
  O = .o
  A = .a
  SO = .dylib
  LIBPRE = lib
else
  ifeq "$(OS)" "linux"
    $(error "Gasp! Linux isn't yet supported!")
  else
    ifeq "$(OS)" "win32"
      CC = cl
      LD = link
      # OPT = -O2 -DNDEBUG
      OPT = -MDd
      PROF =
      CPPFLAGS = -Zi -W3 $(OPT) $(PROF) -TC # -bigobj
      CFLAGS += $(CPPFLAGS)
      LDFLAGS += -nologo -incremental:no -subsystem:console -debug
      X = .exe
      O = .obj
      A = .lib
      SO = .dll
      LIBPRE =
#      VS_VERSION=12.0
#      ifeq "$(VS_VERSION)" "12.0" # This works for VS2013 + Windows 10
#        VS_HOME=/cygdrive/c/Program Files (x86)/Microsoft Visual Studio 12.0
#        WINDOWSSDKDIR=/cygdrive/c/Program Files (x86)/Windows Kits/8.1
#      else # This works for VS2010 + Windows 7
#        VS_HOME=/cygdrive/C/Program Files (x86)/Microsoft Visual Studio 10.0
#        WINDOWSSDKDIR=/cygdrive/C/Program Files (x86)/Microsoft SDKs/Windows/v7.0A
#      endif
    else
      $(error "$(CONFIG): unsupported config")
    endif
  endif
endif

# We're assuming use of cygwin, which means Windows path names can be
# obtained using "cygpath". With "-m" we get slashes (rather than
# backslashes), which all of MS' tools accept and which are far less
# troublesome in make.
ifeq "$(CC)" "cl"
  N_VDDS_HOME := $(shell cygpath -m '$(VDDS_HOME)')
  N_PWD := $(shell cygpath -m '$(PWD)')
  #N_VS_HOME := $(shell cygpath -m '$(VS_HOME)')
  #N_WINDOWSSDKDIR := $(shell cygpath -m '$(WINDOWSSDKDIR)')
else # not Windows
  N_VDDS_HOME := $(VDDS_HOME)
  N_PWD := $(PWD)
endif

# More machine- and platform-specific matters.
ifeq "$(CC)" "cl" # Windows
  ifeq "$(PROC)" "x86_64"
    MACHINE = -machine:X64
  endif
  LDFLAGS += $(MACHINE)
  OBJ_OFLAG = -Fo
  EXE_OFLAG = -out:
  SHLIB_OFLAG = -out:
  CPPFLAGS += -D_CRT_SECURE_NO_WARNINGS
#  ifeq "$(VS_VERSION)" "12.0" # This works for VS2013 + Windows 10
#    CPPFLAGS += '-I$(N_VS_HOME)/VC/include' '-I$(N_WINDOWSSDKDIR)/Include/um' '-I$(N_WINDOWSSDKDIR)/Include/shared'
#    ifeq "$(PROC)" "x86_64"
#      LDFLAGS += '-libpath:$(N_VS_HOME)/VC/lib/amd64' '-libpath:$(N_WINDOWSSDKDIR)/lib/winv6.3/um/x64'
#    else
#      LDFLAGS += '-libpath:$(N_VS_HOME)/VC/lib' '-libpath:$(N_WINDOWSSDKDIR)/lib/winv6.3/um/x86'
#    endif
#  else # This works for VS2010 + Windows 7
#    CPPFLAGS += '-I$(N_VS_HOME)/VC/include' '-I$(N_WINDOWSSDKDIR)/Include'
#    ifeq "$(PROC)" "x86_64"
#      LDFLAGS += '-libpath:$(N_VS_HOME)/VC/lib/amd64' '-libpath:$(N_WINDOWSSDKDIR)/lib/x64'
#    else
#      LDFLAGS += '-libpath:$(N_VS_HOME)/VC/lib' '-libpath:$(N_WINDOWSSDKDIR)/lib'
#    endif
#  endif
else # not Windows
  OBJ_OFLAG = -o
  EXE_OFLAG = -o
  SHLIB_OFLAG = -o
  ifeq "$(PROC)" "x86"
    CFLAGS += -m32
    LDFLAGS += -m32
  endif
  ifeq "$(PROC)" "x86_64"
    CFLAGS += -m64
    LDFLAGS += -m64
  endif
endif

ifeq "$(CC)" "cl"
  LDFLAGS += -libpath:$(N_PWD)/gen
  LIBDEP_SYS = kernel32 ws2_32
else
  LDFLAGS += -L$(N_PWD)/gen
  LIBDEP_SYS =
endif

ifeq "$(OS)" "darwin"
  ifneq "$(findstring clang, $(CC))" ""
    define make_exe
	$(LD) $(LDFLAGS) $(patsubst -L%, -rpath %, $(filter -L%, $(LDFLAGS))) $(EXE_OFLAG)$@ $^ $(LDLIBS)
    endef
    define make_shlib
	$(LD) $(LDFLAGS) $(patsubst -L%, -rpath %, $(filter -L%, $(LDFLAGS))) -dynamiclib -install_name @rpath/$(notdir $@) $(SHLIB_OFLAG)$@ $^ $(LDLIBS)
    endef
  else # assume gcc
    comma=,
    define make_exe
	$(LD) $(LDFLAGS) $(patsubst -L%, -Wl$(comma)-rpath$(comma)%, $(filter -L%, $(LDFLAGS))) $(EXE_OFLAG)$@ $^ $(LDLIBS)
    endef
    define make_shlib
	$(LD) $(LDFLAGS) $(patsubst -L%, -Wl$(comma)-rpath$(comma)%, $(filter -L%, $(LDFLAGS))) -dynamiclib -Wl,-install_name,@rpath/$(notdir $@) $(SHLIB_OFLAG)$@ $^ $(LDLIBS)
    endef
  endif
  define make_archive
	ar -ru $@ $?
  endef
  define make_dep
	$(CC) -M $(CPPFLAGS) $< -o $@
  endef
else
  ifeq "$(OS)" "linux"
    define make_exe
	$(LD) $(LDFLAGS) $(EXE_OFLAG)$@ $^ $(LDLIBS:-l%=%.lib)
    endef
    define make_shlib
	$(LD) $(LDFLAGS) $(patsubst -L%, -Wl$(comma)-rpath %, $(filter -L%, $(LDFLAGS))) -dynamiclib $(SHLIB_OFLAG)$@ $^ $(LDLIBS:-l%=%.lib)
    endef
    define make_archive
	ar -ru $@ $?
    endef
    define make_dep
	$(CC) -M $(CPPFLAGS) $< -o $@
    endef
  else
    ifeq "$(OS)" "win32"
      define make_exe
	$(LD) $(LDFLAGS) $(EXE_OFLAG)$@ $^ $(LDLIBS)
      endef
      define make_shlib
	$(LD) $(LDFLAGS) $(SH_OFLAG)$@ $^ $(LDLIBS)
      endef
      define make_archive
	lib $(MACHINE) /out:$@ $^
      endef
      define make_dep
	$(CC) -E $(CPPFLAGS) $(CPPFLAGS) $< | grep "^#line.*\\\\ospl[io]\\\\" | cut -d '"' -f 2 | sort -u | sed -e 's@\([A-Za-z]\)\:@ /cygdrive/\1@' -e 's@\\\\@/@g' -e '$$!s@$$@ \\@' -e '1s@^@$*$O: @' >$@
      endef
    endif
  endif
endif

%$O:
%$X:
%$(SO):
%.d:
