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
$Id: bbsboa.c,v 1.11 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

int
main ()
{
  char *sec, buf[256], category, DirOfBoard[256], *title,
    filename[STRLEN], *ptr, brc_buf[BRC_MAXSIZE], brc_name[BRC_STRLEN];
  int index, fd, fd1, SectNumber, total, num = 0, all, unread,
    brc_size, brc_num, brc_list[BRC_MAXNUM];
  struct boardheader *buffer;
  struct fileheader DirInfo;
  struct stat st;


  cgi_init ();
  login_check ();
  cgi_html_head ();
  sec = cgi_get ("sec");
  SectNumber = atoi (sec);
  cgi_quit ();

  printf ("<title>%s[%s类]讨论区列表</title>\n", BBSID,
	  SectNames[SectNumber][0]);
  printf ("<body>\n");
  printf ("<center>\n");

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>[讨论区选单]</th>\n");
  printf ("<th class=title width=33%% align=center>%s</th>\n", BBSNAME);
  printf ("<th class=title width=34%% align=right>类别 [%s]</th>\n",
	  SectNames[SectNumber][0]);
  printf ("</table>\n");


  printf ("<hr>\n");

  printf ("<table class=body>\n");
  printf ("<tr><th class=body>序号<th class=body>讨论区名称");
  printf ("<th class=body>更新时间");
  printf ("<th class=body>类别<th class=body>转信<th class=body>中文描述");
  printf ("<th class=body>板主<th class=body>文章数</tr>\n");

  sprintf (buf, "%s/.BOARDS", BBSHOME);
  fd = open (buf, O_RDONLY);
  if (fd == -1)
    show_error ("Can not open .BOARDS file!");

  (void) fstat (fd, &st);
  total = st.st_size / sizeof (struct boardheader);
  buffer =
    (struct boardheader *) calloc ((size_t) total,
				   sizeof (struct boardheader));
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

  for (index = 0; index < total; index++)
    {
      category = buffer[index].title[0];
      if (strchr (CateInSect[SectNumber], category) == NULL)
	continue;
      if (buffer[index].level > 0 && !(buffer[index].level & cookie.level))
	continue;
      sprintf (DirOfBoard, "%s/boards/%s/.DIR", BBSHOME,
	       buffer[index].filename);
      fd1 = open (DirOfBoard, O_RDONLY);
      if (fd1 == -1)
	continue;
      (void) fstat (fd1, &st);
      all = st.st_size / sizeof (DirInfo);
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
      printf ("<a href=\"%s/bbsdoc?board=%s%s\">%s</a>", BBSCGI,
	      buffer[index].filename, cookie_string, buffer[index].filename);
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
      printf ("<td class=body%d align=right>%d\n", (num - 1) % 2 + 1, all);
    }
  free (buffer);
  printf ("</table>\n");

  printf ("<hr>");

  printf ("<table class=foot>\n");
  print_go_bbsman ();
  print_go_bbssec ();
  print_go_bbsall ();
  print_go_bbs0an ();
  print_go_loginout ();
  printf ("</table>\n");

  printf ("</center>\n");
  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
