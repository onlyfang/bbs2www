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
$Id: bbscon.c,v 1.15 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

int
main ()
{
  FILE *fp;
  char *board, *filename, buf[256], tmp[STRLEN], *ptr, brc_buf[BRC_MAXSIZE],
    brc_name[BRC_STRLEN];
  int fd, number, total, title, brc_size, brc_num, brc_list[BRC_MAXNUM],
    finish = 0, i, n, ftime;
  struct fileheader DirInfo;
  struct stat st;


  cgi_init ();
  login_check ();
  cgi_html_head ();
  printf ("<title>%s文章</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  board = cgi_get ("board");
  filename = cgi_get ("file");
  number = atoi (cgi_get ("num"));
  title = atoi (cgi_get ("title"));

  if (strlen (filename) < 5)
    show_error ("Wrong file name!");

  sprintf (buf, "%s/boards/%s/.DIR", BBSHOME, board);
  fd = open (buf, O_RDONLY);
  if (fd == -1)
    show_error ("Can not open .DIR file!");

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>文章閱讀</th>\n");
  printf ("<th class=title width=33%% align=center>%s</th>\n", BBSNAME);
  printf ("<th class=title width=34%% align=right>討論區 [%s]</th>\n", board);
  printf ("</table>\n");

  printf ("<hr>\n");

  (void) fstat (fd, &st);
  total = st.st_size / sizeof (DirInfo);
  if ((number < 1) || (number > total))
    number = total;
  if ((title < 1) || (title > total))
    title = total;

  sprintf (tmp, "%s/boards/%s/%s", BBSHOME, board, filename);

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
    ("<th class=foot><a href=\"%s/bbsdoc?board=%s&go=S&to=%d%s\">本討論區</a>",
     BBSCGI, board, number, cookie_string);

  printf
    ("<th class=foot><a href=\"%s/bbstop?board=%s&go=S&to=%d%s\">主題模式</a>",
     BBSCGI, board, title, cookie_string);


  if (number > 2)
    (void) lseek (fd, (off_t) ((number - 2) * sizeof (DirInfo)), SEEK_SET);
  if (number > 1)
    {
      (void) read (fd, &DirInfo, sizeof (DirInfo));
      printf
	("<th class=foot><a href=\"%s/bbscon?board=%s&file=%s&num=%d%s\">上一篇</a>",
	 BBSCGI, board, DirInfo.filename, number - 1, cookie_string);
    }
  if (number < total)
    {
      (void) lseek (fd, (off_t) sizeof (DirInfo), SEEK_CUR);
      (void) read (fd, &DirInfo, sizeof (DirInfo));
      printf
	("<th class=foot><a href=\"%s/bbscon?board=%s&file=%s&num=%d%s\">下一篇</a>",
	 BBSCGI, board, DirInfo.filename, number + 1, cookie_string);
    }
  (void) read (fd, &DirInfo, sizeof (DirInfo));
  if (strcmp (cookie.id, "guest") != 0)
    printf
      ("<th class=foot><a href=\"%s/bbspst?board=%s&file=%s%s\">回文章</a>",
       BBSCGI, board, filename, cookie_string);
  (void) close (fd);

  brc_num = 0;
  sprintf (buf, "%s/home/%c/%s/.boardrc", BBSHOME, toupper (cookie.id[0]),
	   cookie.id);

  if ((fd = open (buf, O_RDONLY)) != -1)
    {
      brc_size = read (fd, brc_buf, sizeof (brc_buf));
      close (fd);
    }
  else
    brc_size = 0;

  ptr = brc_buf;
  while (ptr < &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= 'z'))
    {
      ptr = brc_getrecord (ptr, brc_name, &brc_num, brc_list);
      if (strncmp (brc_name, board, BRC_STRLEN) == 0)
	break;
    }


  if (strcmp (cookie.id, "guest") != 0 &&
      (filename[0] == 'M' || filename[0] == 'G') && filename[1] == '.' &&
      brc_unread (filename, brc_num, brc_list))
    {
      ptr = brc_buf;
      while (ptr < &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= 'z'))
	{
	  ptr = brc_getrecord (ptr, brc_name, &brc_num, brc_list);
	  if (strncmp (brc_name, board, BRC_STRLEN) == 0)
	    {
	      while (ptr < &brc_buf[brc_size])
		{
		  *(ptr - BRC_ITEMSIZE) = *ptr;
		  ptr++;
		}
	      *(ptr - BRC_ITEMSIZE) = '\0';
	      brc_size -= BRC_ITEMSIZE;
	      break;
	    }
	}

      ftime = atoi (&filename[2]);
      if (brc_num <= 0)
	{
	  brc_list[brc_num++] = ftime;
	  finish = 1;
	}
      else
	{
	  for (n = 0; n < brc_num; n++)
	    if (ftime == brc_list[n])
	      {
		finish = 1;
		break;
	      }
	    else if (ftime > brc_list[n])
	      {
		if (brc_num < BRC_MAXNUM)
		  brc_num++;
		for (i = brc_num - 1; i > n; i--)
		  brc_list[i] = brc_list[i - 1];
		brc_list[n] = ftime;
		finish = 1;
		break;
	      }
	}
      if (!finish && brc_num < BRC_MAXNUM)
	brc_list[brc_num++] = ftime;

      if ((fd = open (buf, O_WRONLY | O_CREAT, 0644)) != -1)
	{
	  strncpy (buf, board, BRC_STRLEN);
	  buf[BRC_STRLEN] = brc_num;
	  memcpy (buf + BRC_STRLEN + 1, brc_list, brc_num * sizeof (int));
	  write (fd, buf, BRC_ITEMSIZE);
	  write (fd, brc_buf, brc_size);
	  close (fd);
	}
    }

  cgi_quit ();

  print_go_loginout ();
  printf ("</table>\n");

  printf ("</center>\n");

  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
