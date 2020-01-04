/* games-find-file.h:

   Copyright 2006 Callum McKenzie

   This library is free software; you can redistribute it and'or modify
   it under the terms of the GNU Library General Public License as published 
   by the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, see <http://www.gnu.org/licenses/>.

   Authors:   Callum McKenzie <callum@spooky-possum.org>
*/

#ifndef GAMES_FIND_FILE_H
#define GAMES_FIND_FILE_H

#include <glib.h>

G_BEGIN_DECLS
  gchar * games_find_similar_file (const gchar * target,
				   const gchar * directory);

G_END_DECLS
#endif
