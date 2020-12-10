#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = g++

# define any compile-time flags

#-pthread -ldl -fPIC
AKAICFLAGS = -Wall -fPIC  -c -fpermissive -Wwrite-strings
# define any directories containing header files other than /usr/include
#
#INCLUDES = -I ./akaiutil
AKAIDIR = akaiutil
CFLAGS = -Wall -std=c++14 -fPIC -I./$(AKAIDIR)
#INCLUDES = "./"

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
#LFLAGS = -L. -lakaiutil
#./akaiutil  
#LFLAGS = ""
# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
LIBS = -static -lwt -lwthttp 

# define the C source files
SRCS = TreeViewDragDrop.c FolderView.c AkaiProgram.c

AKAI_TAR = akaiutil_tar.c akaiutil_tar.h akaiutil_file.h akaiutil_take.h akaiutil.h akaiutil_io.h commoninclude.h
AKAI_FILE = akaiutil_file.c akaiutil_file.h akaiutil_wav.h akaiutil.h commoninclude.h
AKAI_TAKE = akaiutil_take.c akaiutil_take.h akaiutil_wav.h akaiutil.h akaiutil_io.h commoninclude.h
AKAI_WAV = akaiutil_wav.c akaiutil_wav.h commoninclude.h
AKAIUTIL = akaiutil.c akaiutil.h akaiutil_io.h akaiutil_file.h commoninclude.h
AKAI_IO = akaiutil_io.c akaiutil_io.h commoninclude.h
AKAI_COMMON = commonlib.c commoninclude.h

AKAISRC = akaiutil.c \
          akaiutil_file.c \
          akaiutil_io.c \
          akaiutil_main.c \
          akaiutil_take.c \
          akaiutil_tar.c \
          akaiutil_wav.c \
          commonlib.c 
          #\
	  akaiutil_socket.c \
	  akaiutil_main_socket.c \
	  akaisendmessage.c \

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
AKAIOBJS := $(notdir $(patsubst %.c,%.o,$(wildcard $(AKAIDIR)/*.c)))
AKAIOBJS2 =$(addprefix $(AKAIDIR)/,$(AKAIOBJS))
OBJS = $(SRCS:.c=.o) 

 #@echo  $(AKAIOBJS)
 $(info    akaiobjs: $(patsubst %.c,$(AKAIDIR)/%.o,$(wildcard *.c)))
 $(info    objs: $(OBJS))



# define the executable file 
MAIN = afe.wt


#********************build akai lib***********************
AKAI = libakaiutil.a


#  Makefile template for Static library. 
# 1. Compile every *.cpp in the folder 
# 2. All obj files under obj folder
# 3. static library .a at lib folder
# 4. run 'make dirmake' before calling 'make'




# INC = -I./akaiutil


OUT_DIR=.

# Enumerating of every *.cpp as *.o and using that as dependency.	
# filter list of .c files in a directory.
# FILES =dump_l.c \
#	kter.c \
#
# $(AKAI): $(patsubst %.c,$(AKAIOBJS2)/%.o,$(wildcard $(FILES))) 
$(info akaiobjs: $(addprefix $(AKAIDIR)/,$(notdir $(patsubst %.c,%.o,$(wildcard $(AKAIDIR)/*.c)))))

# Enumerating of every *.c as *.o and using that as dependency
$(AKAI): $(addprefix $(AKAIDIR)/,$(notdir $(patsubst %.c,%.o,$(wildcard $(AKAIDIR)/*.c))))
	ar -r -o $(OUT_DIR)/$@ $^
	# libtool --mode=link cc -static -o $(OUT_DIR)/$@ $^




#Compiling every *.c to *.o
$(AKAIDIR)/%.o: %.c 
	$(CC) -c $(INC) $(AKAICFLAGS) -o $@  $<
	
dirmake:
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(AKAIOBJS2)
	
# clean:
# 	rm -f $(AKAIOBJS2)/*.o $(OUT_DIR)/$(AKAI)

rebuild: clean build



#***************************Build gui program*************************************



#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(AKAI) $(MAIN) 
	@echo  akaiutil GUI has been compiled.

$(MAIN): $(OBJS) $(AKAI)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(AKAI) $(LFLAGS) $(LIBS)
	
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)
	$(RM) $(AKAIDIR)/*.o *~ libakaiutil.a

depend: $(SRCS)
	makedepend $(INCLUDES) $^

#DO NOT DELETE THIS LINE -- make depend needs it



