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
$Id: bbsanc.c,v 1.10 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

int
main ()
{
  FILE *fp;
  char buf[256], *path;

  cgi_init ();
  login_check ();
  cgi_html_head ();

  printf ("<title>%s精華區文章</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");
  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>文章閱讀</th>\n");
  printf ("<th class=title width=33%% align=center>%s</th>\n", BBSNAME);
  printf ("<th class=title width=34%% align=right>精華區</th>\n");
  printf ("</table>\n");

  printf ("<hr>\n");

  path = cgi_get ("path");

  sprintf (buf, "%s/0Announce%s", BBSHOME, path);

  cgi_quit ();

  if ((fp = fopen (buf, "r")) == NULL)
    show_error ("Can not open file!\n");

  printf ("<table class=doc>");
  printf ("<tr><td class=doc>");
  display_article (fp, 0);
  printf ("</table>\n");

  (void) fclose (fp);
  printf ("<hr>\n");
  printf ("<table class=foot>\n");
  print_go_bbsman ();
  print_go_bbssec ();
  print_go_bbsall ();
  print_go_loginout ();
  printf ("</table>\n");

  printf ("</center>\n");
  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
