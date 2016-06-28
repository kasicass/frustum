 #
 # The 3D Studio File Format Library
 # Copyright (C) 1996-2001 by J.E. Hoffmann <je-h@gmx.net>
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
 # $Id: Makefile.am,v 1.9 2001/06/16 16:26:20 jeh Exp $
 #

lib3dsdir=$(includedir)/lib3ds

INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES = lib3ds.la

lib3ds_la_SOURCES = \
  readwrite.c \
  float.c \
  vector.c \
  matrix.c \
  quat.c \
  tcb.c \
  ease.c \
  chunktable.h \
  chunk.c \
  file.c \
  background.c \
  atmosphere.c \
  shadow.c \
  viewport.c \
  material.c \
  mesh.c \
  camera.c \
  light.c \
  tracks.c \
  node.c

lib3ds_HEADERS = \
  types.h \
  readwrite.h \
  float.h \
  vector.h \
  matrix.h \
  quat.h \
  tcb.h \
  ease.h \
  chunk.h \
  file.h \
  background.h \
  atmosphere.h \
  shadow.h \
  viewport.h \
  material.h \
  mesh.h \
  camera.h \
  light.h \
  tracks.h \
  node.h

EXTRA_DIST = \
  types.txt \
  chunktable.sed

chunktable.h: chunk.h chunktable.sed
	sed -f chunktable.sed chunk.h >chunktable.h

chunk.c: chunktable.h

lib3ds_la_LDFLAGS= \
 -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
 -release $(LT_RELEASE)

lib3ds_la_LIBADD= \
  -lm
