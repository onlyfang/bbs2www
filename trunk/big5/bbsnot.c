
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
$Id: bbsnot.c,v 1.11 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

int
main ()
{
  FILE *fp;
  char *board, buf[STRLEN];
  int number;

  cgi_init ();
  login_check ();
  cgi_html_head ();
  printf ("<title>%s備忘錄</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  board = cgi_get ("board");
  number = atoi (cgi_get ("num"));

  sprintf (buf, "%s/boards/%s/.DIR", BBSHOME, board);
  if (!dashf (buf))
    show_error ("Can not open .DIR file!");

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>備忘錄</th>\n");
  printf ("<th class=title width=33%% align=center>%s</th>\n", BBSNAME);
  printf ("<th class=title width=34%% align=right>討論區 [%s]</th>\n", board);
  printf ("</table>\n");

  printf ("<hr>\n");

  sprintf (buf, "%s/vote/%s/notes", BBSHOME, board);

  if ((fp = fopen (buf, "r")) == NULL)
    printf ("<p>此討論區尚無「備忘錄」。</p>\n");
  else
    {
      printf ("<table class=doc>");
      printf ("<tr><td class=doc>");

      display_article (fp, 1);

      printf ("</table>\n");

      (void) fclose (fp);
    }

  printf ("<hr>\n");

  printf ("<table class=foot>\n");
  print_go_bbsman ();

  printf
    ("<th class=foot><a href=\"%s/bbsdoc?board=%s&go=S&to=%d%s\">本討論區</a>",
     BBSCGI, board, number, cookie_string);
  if (strcmp (cookie.id, "guest") != 0)
    printf
      ("<th class=foot><a href=%s/bbspst?board=%s%s>IOA</a>\n",
       BBSCGI, board, cookie_string);

  print_go_loginout ();
  printf ("</table>\n");

  cgi_quit ();
  printf ("</center>\n");

  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
