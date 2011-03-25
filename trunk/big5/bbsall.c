
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
$Id: bbsall.c,v 1.10 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

static int
namesort (const void *board1, const void *board2)
{
  struct boardheader *b1, *b2;

  b1 = (struct boardheader *) board1;
  b2 = (struct boardheader *) board2;
  return (strcasecmp (b1->filename, b2->filename));
}

int
main ()
{
  char filename[256], *title, *ptr, buf[256],
    brc_buf[BRC_MAXSIZE], brc_name[BRC_STRLEN];
  int fd, fd1, num, index = 0, total = 0, count,
    unread, brc_size, brc_num, brc_list[BRC_MAXNUM];
  struct boardheader *buffer;
  struct stat st;

  cgi_init ();
  login_check ();
  cgi_html_head ();
  cgi_quit ();
  printf ("<title>%s全部討論區列表</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>全部討論區</th>\n");
  printf ("<th class=title width=33%% align=center>%s</th>\n", BBSNAME);
  printf ("<th class=title width=34%% align=right>域名 [%s]</th>\n", BBSHOST);
  printf ("</table>\n");


  printf ("<hr>\n");

  printf ("<table class=body>\n");
  printf ("<tr><th class=body>序號<th class=body>討論區名稱");
  printf ("<th class=body>更新時間");
  printf ("<th class=body>類別<th class=body>轉信<th class=body>");
  printf ("中文描述<th class=body>板主<th class=body>文章數</tr>\n");

  sprintf (buf, "%s/.BOARDS", BBSHOME);
  fd = open (buf, O_RDONLY);
  if (fd == -1)
    show_error ("Can not open .BOARDS file!");

  (void) fstat (fd, &st);
  total = st.st_size / sizeof (struct boardheader);
  buffer = (struct boardheader *)
    calloc ((size_t) total, sizeof (struct boardheader));
  if (buffer == NULL)
    show_error ("Out of memory!");

  if (read (fd, buffer, (size_t) st.st_size) < (ssize_t) st.st_size)
    show_error ("Can not read .BOARDS file!");

  (void) close (fd);

  sprintf (buf, "%s/home/%c/%s/.boardrc", BBSHOME,
	   toupper (cookie.id[0]), cookie.id);

  if ((fd = open (buf, O_RDONLY)) != -1)
    {
      brc_size = read (fd, brc_buf, sizeof (brc_buf));
      close (fd);
    }
  else
    brc_size = 0;

  qsort (buffer, (size_t) total, sizeof (struct boardheader), namesort);
  num = 0;
  for (index = 0; index < total; index++)
    {
      if (buffer[index].level > 0 && !(buffer[index].level & cookie.level))
	continue;
      sprintf (buf, "%s/boards/%s/.DIR", BBSHOME, buffer[index].filename);
      fd1 = open (buf, O_RDONLY);
      if (fd1 == -1)
	continue;
      (void) fstat (fd1, &st);
      count = st.st_size / sizeof (struct fileheader);
      unread = 1;
      lseek (fd1, (off_t) (-1 * sizeof (struct fileheader)), SEEK_END);
      if (read (fd1, filename, STRLEN) > 0
	  && strcmp (cookie.id, "guest") != 0)
	{
	  brc_num = 0;
	  ptr = brc_buf;
	  while (ptr < &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= 'z'))
	    {
	      ptr = brc_getrecord (ptr, brc_name, &brc_num, brc_list);
	      if (strncmp (brc_name, buffer[index].filename, BRC_STRLEN) == 0
		  && !brc_unread (filename, brc_num, brc_list))
		{
		  unread = 0;
		  break;
		}
	    }
	}
      (void) close (fd1);
      num++;

      printf ("<tr><td class=body%d>", (num - 1) % 2 + 1);
      printf ("%d<td class=body%d>", num, (num - 1) % 2 + 1);
      if (unread)
	printf ("◆ ");
      else
	printf ("◇ ");
      printf ("<a href=\"%s/bbsdoc?board=%s%s\">%s</a>",
	      BBSCGI, buffer[index].filename, cookie_string,
	      buffer[index].filename);
      title = buffer[index].title + 1;
      title[6] = '\0';
      printf ("<td class=body%d>%12.12s", (num - 1) % 2 + 1,
	      ctime (&st.st_mtime) + 4);
      printf ("<td class=body%d>%s", (num - 1) % 2 + 1, title);
      title += 7;
      title[2] = '\0';
      printf ("<td class=body%d align=center>%s", (num - 1) % 2 + 1, title);
#ifdef FB25
      title += 5;
#else
      title += 3;
#endif
      printf ("<td class=body%d><a href=\"%s/bbstop?board=%s%s\">%s</a>",
	      (num - 1) % 2 + 1, BBSCGI, buffer[index].filename,
	      cookie_string, title);
      printf ("<td class=body%d>", (num - 1) % 2 + 1);
      print_user (buffer[index].BM);
      printf ("<td class=body%d align=right>%d\n", (num - 1) % 2 + 1, count);
    }
  free (buffer);
  printf ("</table>\n");

  printf ("<hr>\n");

  printf ("<table class=foot>\n");
  print_go_bbsman ();
  print_go_bbssec ();
  print_go_bbs0an ();
  print_go_loginout ();
  printf ("</table>\n");

  printf ("</center>\n");
  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
