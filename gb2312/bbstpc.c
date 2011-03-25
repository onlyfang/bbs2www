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
$Id: bbstpc.c,v 1.5 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

int
main ()
{
  char *board, *filename, buf[256], type, *date, *ptr, brc_buf[BRC_MAXSIZE],
    brc_name[BRC_STRLEN];
  int fd, i, n, number, index, total, found, brc_size, brc_num,
    brc_list[BRC_MAXNUM];
  struct fileheader *buffer;
  struct stat st;
  time_t filetime;
#ifdef COLOR_POST_DATE
  struct tm *mytm;
#endif



  cgi_init ();
  login_check ();
  cgi_html_head ();
  board = cgi_get ("board");
  filename = cgi_get ("file");
  number = atoi (cgi_get ("num"));

  printf ("<title>%s文章</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");
  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>同主题文章阅读</th>\n");
  printf ("<th class=title width=33%% align=center>%s</th>\n", BBSNAME);
  printf ("<th class=title width=34%% align=right>讨论区 [%s]</th>\n", board);
  printf ("</table>\n");

  printf ("<hr>\n");

  sprintf (buf, "%s/boards/%s/.DIR", BBSHOME, board);
  fd = open (buf, O_RDONLY);
  if (fd == -1)
    show_error ("Can not open .DIR file!");

  (void) fstat (fd, &st);
  total = st.st_size / sizeof (struct fileheader);

  buffer = (struct fileheader *) calloc (total, sizeof (struct fileheader));
  if (buffer == NULL)
    show_error ("Out of memory!");

  if (read (fd, buffer, (size_t) st.st_size) < (ssize_t) st.st_size)
    {
      free (buffer);
      show_error ("Can not read .DIR file!");
    }
  (void) close (fd);

  n = 0;
  while (n < total)
    {
      if (strcmp (buffer[n].filename, filename) == 0)
	break;
      else
	n++;
    }


  printf ("<table class=body>\n");
  printf ("<tr><th class=body>序号<th class=body>状态<th class=body>作者");
  printf ("<th class=body>日期<th class=body>标题\n");

  brc_num = 0;
  sprintf (filename, "%s/home/%c/%s/.boardrc", BBSHOME,
	   toupper (cookie.id[0]), cookie.id);

  if ((fd = open (filename, O_RDONLY)) != -1)
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

  i = 1;
  for (index = n; index < total; index++)
    {
      found = 0;
      if ((strncmp ("Re: ", buffer[index].title, 4) != 0) &&
	  (strncmp ("RE: ", buffer[index].title, 4) != 0))
	{
	  if (strcmp (buffer[index].title, buffer[n].title) == 0)
	    found = 1;
	}
      else
	{
	  if (strcmp (buffer[index].title + 4, buffer[n].title) == 0)
	    found = 1;
	}
      if (found)
	{
	  type = ' ';
	  printf ("<tr><td class=body%d>%d<td class=body%d>", i % 2 + 1,
		  i, i % 2 + 1);

	  if (brc_unread (buffer[index].filename, brc_num, brc_list))
	    type = 'N';
	  else
	    type = ' ';
	  if (buffer[index].accessed[0] & FILE_DIGEST)
	    {
	      if (type == ' ')
		type = 'g';
	      else
		type = 'G';
	    }
	  if (buffer[index].accessed[0] & FILE_MARKED)
	    {
	      switch (type)
		{
		case ' ':
		  type = 'm';
		  break;
		case 'N':
		  type = 'M';
		  break;
		case 'g':
		  type = 'b';
		  break;
		case 'G':
		  type = 'B';
		  break;
		}
	    }

	  if (type != ' ' && type != 'N')
	    printf ("<font class=col31>%c</font>", type);
	  else
	    printf ("%c", type);

	  printf ("<td class=body%d>", i % 2 + 1);
	  print_user (buffer[index].owner);

	  filetime = (time_t) atoi (buffer[index].filename + 2);
	  if (filetime > 740000000)
	    date = ctime (&filetime) + 4;
	  else
	    date = " ";

	  printf ("<td class=body%d>", i % 2 + 1);
#ifdef COLOR_POST_DATE
	  mytm = localtime (&filetime);
	  printf ("<font class=col3%d>%6.6s</font>", mytm->tm_wday + 1, date);
#else
	  printf ("%6.6s", date);
#endif
	  printf ("<td class=body%d>", i % 2 + 1);
	  printf ("<a href=\"%s/bbscon?board=%s&file=%s&num=%d&title=%d%s\">",
		  BBSCGI, board, buffer[index].filename, index + 1, number,
		  cookie_string);
	  if (strncmp ("Re:", buffer[index].title, 3) != 0
	      && strncmp ("RE:", buffer[index].title, 3) != 0)
	    printf ("● ");
	  print_title (buffer[index].title);
	  printf ("</a>\n");
	  i++;
	}
    }



  printf ("</table>\n");

  printf ("<hr>\n");

  printf ("<table class=foot>\n");
  print_go_bbsman ();

  printf
    ("<th class=foot><a href=\"%s/bbsdoc?board=%s&go=S&to=%d%s\">本讨论区</a>",
     BBSCGI, board, n + i, cookie_string);

  printf
    ("<th class=foot><a href=\"%s/bbstop?board=%s&go=S&to=%d%s\">主题模式</a>\n",
     BBSCGI, board, number, cookie_string);

  if (strcmp (cookie.id, "guest") != 0)
    printf
      ("<th class=foot><a href=\"%s/bbspst?board=%s&title=%s%s\">发表文章</a>",
       BBSCGI, board, buffer[n].title, cookie_string);
  print_go_loginout ();
  free (buffer);
  cgi_quit ();

  printf ("</table>\n");

  printf ("</center>\n");

  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
