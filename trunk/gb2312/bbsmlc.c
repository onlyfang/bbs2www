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
$Id: bbsmlc.c,v 1.3 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

int
main ()
{
  FILE *fp;
  char *filename, buf[256], tmp[STRLEN];
  int fd, number, total;
  struct fileheader DirInfo;
  struct stat st;


  cgi_init ();
  login_check ();
  if (strcmp (cookie.id, "guest") == 0)
    {
      cgi_quit ();
      printf ("Location: http://%s%s/bbslog?status=3\n\n", BBSHOST, BBSCGI);
      exit (1);
    }
  cgi_html_head ();
  printf ("<title>%s邮件阅读</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  filename = cgi_get ("file");
  number = atoi (cgi_get ("num"));

  if (strlen (filename) < 5)
    show_error ("Wrong file name!");

  sprintf (buf, "%s/mail/%c/%s/.DIR", BBSHOME, toupper (cookie.id[0]),
	   cookie.id);

  fd = open (buf, O_RDWR);
  if (fd == -1)
    show_error ("Can not open .DIR file!");

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>邮件阅读</th>\n");
  printf ("<th class=title width=33%% align=center>%s</th>\n", BBSNAME);
  printf ("<th class=title width=34%% align=right>用户名 [%s]</th>\n",
	  cookie.id);
  printf ("</table>\n");

  printf ("<hr>\n");

  (void) fstat (fd, &st);
  total = st.st_size / sizeof (DirInfo);
  if ((number < 1) || (number > total))
    number = total;

  sprintf (tmp, "%s/mail/%c/%s/%s", BBSHOME, toupper (cookie.id[0]),
	   cookie.id, filename);

  if ((fp = fopen (tmp, "r")) == NULL)
    show_error ("Can not open required file!");

  printf ("<table class=doc>");
  printf ("<tr><td class=doc>");

  display_article (fp, 0);

  printf ("</table>\n");

  (void) fclose (fp);

  printf ("<hr>\n");

  printf ("<table class=foot>\n");
  print_go_bbsman ();

  printf
    ("<th class=foot><a href=\"%s/bbsmil?go=S&to=%d%s\">返回信箱</a>",
     BBSCGI, number, cookie_string);

  lockfile (fd, 1);
  if (number > 2)
    (void) lseek (fd, (off_t) ((number - 2) * sizeof (DirInfo)), SEEK_SET);
  if (number > 1)
    {
      (void) read (fd, &DirInfo, sizeof (DirInfo));
      printf
	("<th class=foot><a href=\"%s/bbsmlc?file=%s&num=%d%s\">上一封</a>",
	 BBSCGI, DirInfo.filename, number - 1, cookie_string);
    }


  (void) read (fd, &DirInfo, sizeof (DirInfo));
  if (!(DirInfo.accessed[0] & FILE_READ) &&
      strcmp (cookie.id, "guest") != 0 &&
      (filename[0] == 'M' || filename[0] == 'G') && filename[1] == '.')
    {
      (void) lseek (fd, (off_t) (-1 * sizeof (DirInfo)), SEEK_CUR);
      DirInfo.accessed[0] |= FILE_READ;
      (void) write (fd, &DirInfo, sizeof (DirInfo));
    }

  if (number < total)
    {
      (void) read (fd, &DirInfo, sizeof (DirInfo));
      printf
	("<th class=foot><a href=\"%s/bbsmlc?file=%s&num=%d%s\">下一封</a>",
	 BBSCGI, DirInfo.filename, number + 1, cookie_string);
    }
  if (strcmp (cookie.id, "guest") != 0)
    printf
      ("<th class=foot><a href=\"%s/bbspsm?file=%s%s\">回信</a>",
       BBSCGI, filename, cookie_string);

  lockfile (fd, 0);
  (void) close (fd);


  cgi_quit ();

  print_go_loginout ();
  printf ("</table>\n");

  printf ("</center>\n");

  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
