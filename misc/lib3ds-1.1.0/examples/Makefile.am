 #
 # The 3D Studio File Format Library
 # Copyright (C) 1996-2000 by J.E. Hoffmann <je-h@gmx.net>
 # All rights reserved.
 #
 # This program is  free  software;  you can redistribute it and/or modify it
 # under the terms of the  GNU Lesser General Public License  as published by 
 # the  Free Software Foundation;  either version 2.1 of the License,  or (at 
 # your option) any later version.
 #
 # This  program  is  distributed in  the  hope that it will  be useful,  but
 # WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 # or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public  
 # License for more details.
 #
 # You should  have received  a copy of the GNU Lesser General Public License
 # along with  this program;  if not, write to the  Free Software Foundation,
 # Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 #
 # $Id: Makefile.am,v 1.11 2001/06/16 17:06:52 jeh Exp $
 #

INCLUDES = \
  -I$(top_srcdir) \
  @DMALLOC_CFLAGS@

bin_PROGRAMS = \
  lib3dsdump \
  lib3ds2m \
  lib3ds2rib \
  lib3dsplay

lib3dsdump_SOURCES = 3dsdump.c
lib3dsdump_LDADD = \
  $(top_builddir)/lib3ds/lib3ds.la \
  @DMALLOC_LIBS@

lib3ds2m_SOURCES = 3ds2m.c
lib3ds2m_LDADD = \
  $(top_builddir)/lib3ds/lib3ds.la \
  @DMALLOC_LIBS@

lib3ds2rib_SOURCES = 3ds2rib.c
lib3ds2rib_LDADD = \
  $(top_builddir)/lib3ds/lib3ds.la \
  @DMALLOC_LIBS@

lib3dsplay_SOURCES = 3dsplay.c
lib3dsplay_LDADD = \
  $(top_builddir)/lib3ds/lib3ds.la \
  @DMALLOC_LIBS@ \
  -lglut -lGLU -lGL

MANPAGES = \
  3dsdump.1 \
  3ds2m.1 \
  3ds2rib.1 \
  3dsplay.1

man_MANS = $(MANPAGES)
EXTRA_DIST = $(MANPAGES) 

install-exec-hook:
	mv $(DESTDIR)$(bindir)/lib3dsdump $(DESTDIR)$(bindir)/3dsdump
	mv $(DESTDIR)$(bindir)/lib3ds2m $(DESTDIR)$(bindir)/3ds2m
	mv $(DESTDIR)$(bindir)/lib3ds2rib $(DESTDIR)$(bindir)/3ds2rib
	mv $(DESTDIR)$(bindir)/lib3dsplay $(DESTDIR)$(bindir)/3dsplay



