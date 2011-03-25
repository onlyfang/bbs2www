/*
   BBS2WWW -- WWW Interface for Firebird Bulletin Board System
   Copyright (C) 1996-2001 Computer Application Studio.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
$Id: bbsout.c,v 1.2 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

int
main ()
{
  char filename[256];
  int i;
  struct user_info *uin;

  cgi_init ();
  login_check ();
  cgi_quit ();

  if (strcmp (cookie.id, "guest") == 0)
    {
      printf ("Location: %s\n\n", BBSURL);
      return (1);
    }


  for (i = 0; i < USHM_SIZE; i++)
    {
      uin = &(utmpshm->uinfo[i]);
      if (uin->active && uin->mode == WWW
	  && strcmp (uin->userid, cookie.id) == 0)
	{
	  memset (uin, 0, sizeof (struct user_info));
	  break;
	}
    }

  sprintf (filename, "%s/home/%c/%s/.wwwlogin", BBSHOME,
	   toupper (cookie.id[0]), cookie.id);
  (void) remove (filename);

  printf ("Location: %s\n\n", BBSURL);
  return (0);
}
