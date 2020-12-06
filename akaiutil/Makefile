# Copyright (c) 2008-2020 Klaus Michael Indlekofer. All rights reserved.
#
# m.indlekofer@gmx.de
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.



VERSION	=	4.4.2



CFLAGS	+=	-Wall -Wextra 
#-O2
ifdef LINUX
CFLAGS	+=	-D_FILE_OFFSET_BITS=64
endif



all:	akaiutil

install:
	$(INSTALL_PROGRAM) akaiutil $(PREFIX)/bin/

clean:
	rm -f akaiutil akaiutil.exe *.o *.obj

back:
	mkdir -p akaiutil-$(VERSION);\
	cp -p Makefile Makefile.nmake README.txt gpl*.txt *.c *.h *.vcproj *.sln *.dsw *.dsp akaiutil-$(VERSION);\
	tar -clvpzf akaiutil-$(VERSION).tar.gz akaiutil-$(VERSION)

akaiutil:	akaiutil_main.o akaiutil_tar.o akaiutil_file.o akaiutil_take.o akaiutil_wav.o akaiutil.o akaiutil_io.o commonlib.o
	$(CC) $(CFLAGS) -o $@ akaiutil_main.o akaiutil_tar.o akaiutil_file.o akaiutil_take.o akaiutil_wav.o akaiutil.o akaiutil_io.o commonlib.o -lm

akaiutil_main.o:	akaiutil_main.c akaiutil.h akaiutil_io.h akaiutil_tar.h akaiutil_file.h akaiutil_take.h commoninclude.h
	$(CC) $(CFLAGS) -c akaiutil_main.c

akaiutil_tar.o:	akaiutil_tar.c akaiutil_tar.h akaiutil_file.h akaiutil_take.h akaiutil.h akaiutil_io.h commoninclude.h
	$(CC) $(CFLAGS) -c akaiutil_tar.c

akaiutil_file.o:	akaiutil_file.c akaiutil_file.h akaiutil_wav.h akaiutil.h commoninclude.h
	$(CC) $(CFLAGS) -c akaiutil_file.c

akaiutil_take.o:	akaiutil_take.c akaiutil_take.h akaiutil_wav.h akaiutil.h akaiutil_io.h commoninclude.h
	$(CC) $(CFLAGS) -c akaiutil_take.c

akaiutil_wav.o:	akaiutil_wav.c akaiutil_wav.h commoninclude.h
	$(CC) $(CFLAGS) -c akaiutil_wav.c

akaiutil.o:	akaiutil.c akaiutil.h akaiutil_io.h akaiutil_file.h commoninclude.h
	$(CC) $(CFLAGS) -c akaiutil.c

akaiutil_io.o:	akaiutil_io.c akaiutil_io.h commoninclude.h
	$(CC) $(CFLAGS) -c akaiutil_io.c

commonlib.o:	commonlib.c commoninclude.h
	$(CC) $(CFLAGS) -c commonlib.c



# EOF
