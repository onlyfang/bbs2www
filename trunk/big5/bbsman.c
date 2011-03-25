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
$Id: bbsman.c,v 1.3 2001/02/23 21:29:00 Only Exp $
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
  printf ("<title>%s 主選單</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>[主選單]</th>\n");
  printf ("<th class=title width=33%% align=center>%s</th>\n", BBSNAME);
  printf ("<th class=title width=34%% align=right>域名 [%s]</th>\n", BBSHOST);
  printf ("</table>\n");

  printf ("<hr>\n");

  printf ("<table class=body>\n");
  printf ("<tr><th class=body>序號<th class=body>功能\n");
  printf
    ("<tr><td class=body1>1</td><td class=body1><a href=\"%s/bbssec?%s\">分類討論區選單</a>\n",
     BBSCGI, cookie_string);
  printf
    ("<tr><td class=body2>2</td><td class=body2><a href=\"%s/bbsall?%s\">全部討論區列表</a>\n",
     BBSCGI, cookie_string);
  printf
    ("<tr><td class=body1>3</td><td class=body1><a href=\"%s/bbs0an?%s\">精華區公布欄</a>\n",
     BBSCGI, cookie_string);
  index = 3;
  if (strcmp (cookie.id, "guest") != 0)
    {
      printf
	("<tr><td class=body%d>%d</td><td class=body%d><a href=\"%s/bbsmil?%s\">處理郵件</a>\n",
	 index % 2 + 1, index + 1, index % 2 + 1, BBSCGI, cookie_string);
    }
  index++;
  printf
    ("<tr><td class=body%d>%d</td><td class=body%d><a href=\"%s/bbsusr?%s\">環顧四方</a>\n",
     index % 2 + 1, index + 1, index % 2 + 1, BBSCGI, cookie_string);
  index++;
  printf
    ("<tr><td class=body%d>%d</td><td class=body%d><a href=\"%s/bbsqry?%s\">查詢网友</a>\n",
     index % 2 + 1, index + 1, index % 2 + 1, BBSCGI, cookie_string);
  printf ("</table>\n");

  printf ("<hr>\n");

  printf ("<table class=foot>\n");
  print_go_bbssec ();
  print_go_bbsall ();
  print_go_bbs0an ();
  print_go_bbsmil ();
  print_go_loginout ();
  printf ("</table>\n");

  printf
    ("<p><a href=\"http://cas.tsx.org/\"><img src=%sbbs2www20.gif alt=\"BBS2WWW Powered\" border=0></a>",
     BBSURL);
  printf
    ("本站系統來源于/更改自<a href=\"http://cas.tsx.org/\">計算机應用工作室</a>之BBS2WWW 2.0");
  printf ("</center>\n");
  printf ("</body>\n");
  printf ("</html>");

  return (0);

}
