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
$Id: bbssec.c,v 1.9 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

int
main ()
{
  int index;

  cgi_init ();
  login_check ();
  cgi_quit ();
  cgi_html_head ();
  printf ("<title>%s分类讨论区</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>分类讨论区</th>\n");
  printf ("<th class=title width=33%% align=center>%s</th>\n", BBSNAME);
  printf ("<th class=title width=34%% align=right>域名 [%s]</th>\n", BBSHOST);
  printf ("</table>\n");

  printf ("<hr>\n");

  printf ("<table class=body>\n");
  printf ("<tr><th class=body>序号<th class=body>类别<th class=body>描述\n");
  for (index = 0; index < SectNum; index++)
    {
      printf ("<tr><td class=body%d>%d", index % 2 + 1, index + 1);
      printf ("<td class=body%d><a href=\"%s/bbsboa?sec=%d%s\">",
	      index % 2 + 1, BBSCGI, index, cookie_string);
      printf ("%s</a>", SectNames[index][0]);
      printf ("<td class=body%d><a href=\"%s/bbsboa?sec=%d%s\">%s</a>\n",
	      index % 2 + 1, BBSCGI, index, cookie_string,
	      SectNames[index][1]);
    }
  printf ("</table>\n");

  printf ("<hr>\n");

  printf ("<table class=foot>\n");
  print_go_bbsman ();
  print_go_bbsall ();
  print_go_bbs0an ();
  print_go_loginout ();
  printf ("</table>\n");

  printf ("</center>\n");
  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
