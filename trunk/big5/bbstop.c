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
$Id: bbstop.c,v 1.6 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

int
main ()
{
  FILE *fn;
  char buf[256], ch, *board, *go, *to, *ptr, filename[256], category,
    *date, path[STRLEN];
  int i, index, fd, CheckPerm = 0, number = 0, found = 0,
    start = 0, end = 0, total = 0;
  struct fileheader *buffer;
  struct boardheader BoardInfo;
  struct stat st;
  time_t filetime;
#ifdef COLOR_POST_DATE
  struct tm *mytm;
#endif


  cgi_init ();
  login_check ();

  board = cgi_get ("board");
  go = cgi_get ("go");
  to = cgi_get ("to");

  if (strlen (go) > 0)
    ch = go[0];
  else
    ch = 'N';

  if (ch == 'T' || ch == 'W' || ch == 'C' || ch == 'B')
    {
      printf ("Location: http://%s%s/bbsdoc?board=%s&go=%c&to=%s%s\n\n",
	      BBSHOST, BBSCGI, board, ch, to, cookie_string);
      cgi_quit ();
      return (0);
    }
  else if ((ch == 'S') || (ch == 'U') || (ch == 'D'))
    number = atoi (to);

  cgi_html_head ();
  printf ("<title>%s討論區同主題文章列表</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  sprintf (filename, "%s/.BOARDS", BBSHOME);
  fd = open (filename, O_RDONLY);
  if (fd == -1)
    show_error ("Can not open .BOARDS file!");

  while (read (fd, &BoardInfo, sizeof (BoardInfo))
	 == (ssize_t) sizeof (BoardInfo))
    {
      if (strncasecmp (board, BoardInfo.filename, STRLEN) == 0)
	{
	  if (BoardInfo.level > 0 && !(BoardInfo.level & cookie.level))
	    found = 0;
	  else
	    {
	      found = 1;
	      category = BoardInfo.title[0];
	      strcpy (board, BoardInfo.filename);
	    }
	  break;
	}
    }
  (void) close (fd);

  printf ("<form action=\"%s/bbstop\">\n", BBSCGI);

  if (found == 0)
    {
      sprintf (buf, "討論區%s不存在或需要特殊權限!\n", board);
      show_error (buf);
    }

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>");
  if (BoardInfo.BM[0] != '\0')
    {
      printf ("板主 [");
      print_user (BoardInfo.BM);
      printf ("]");
    }
  else
    printf ("誠徵板主中");
  printf ("</th>\n<th class=title width=33%% align=center>%s</th>\n",
	  BBSNAME);
  printf ("<th class=title width=34%% align=right>討論區 [%s]</th>\n", board);
  printf ("</table>\n");

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

  end = total;
  total = 0;
  for (index = 0; index < end; index++)
    {
      if ((strncmp ("Re: ", buffer[index].title, 4) != 0) &&
	  (strncmp ("RE: ", buffer[index].title, 4) != 0))
	{
	  found = 0;
	  for (i = 0; i < total; i++)
	    if (strcmp (buffer[i].title, buffer[index].title) == 0)
	      {
		found = 1;
		buffer[i].accessed[0]++;
		break;
	      }
	  if (!found)
	    {
	      memcpy ((void *) (buffer + total), (void *) (buffer + index),
		      sizeof (struct fileheader));
	      buffer[total].accessed[0] = 1;
	      total++;
	    }
	}
      else
	{
	  for (i = 0; i < total; i++)
	    if (strcmp (buffer[i].title, buffer[index].title + 4) == 0)
	      {
		found = 1;
		buffer[i].accessed[0]++;
		break;
	      }
	}
    }

  printf ("<hr>\n");

  if (total == 0 && (ch == 'N'))
    {
      printf ("本討論區目前沒有文章\n");
    }
  else
    {
      switch (ch)
	{
	case 'U':
	  start = number - 20;
	  if (start < 1)
	    start = 1;
	  end = start + 19;
	  if (end > total)
	    end = total;
	  break;
	case 'D':
	  end = number + 20;
	  if (end > total)
	    end = total;
	  start = end - 19;
	  if (start < 1)
	    start = 1;
	  break;
	case 'A':
	  start = 1;
	  end = total;
	  break;
	case 'S':
	  if (number < 11)
	    {
	      start = 1;
	      end = 20;
	      if (end > total)
		end = total;
	    }
	  else if (number > total - 10)
	    {
	      end = total;
	      start = end - 19;
	      if (start < 1)
		start = 1;
	    }
	  else
	    {
	      start = number - 10;
	      end = start + 19;
	    }
	  break;
	default:
	  end = total;
	  start = total - 19;
	  if (start < 1)
	    start = 1;
	}

      printf ("<table class=body>\n");
      printf ("<tr><th class=body>序號<th class=body>作者");
      printf
	("<th class=body>日期<th class=body>標題<th class=body>文章數\n");

      i = 0;
      for (index = start; index <= end; index++)
	{
	  printf ("<tr><td class=body%d>%d", i % 2 + 1, index);
	  printf ("<td class=body%d>", i % 2 + 1);
	  print_user (buffer[index - 1].owner);

	  filetime = (time_t) atoi (buffer[index - 1].filename + 2);
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
	  printf ("<a href=\"%s/bbstpc?board=%s&file=%s&num=%d%s\">",
		  BBSCGI, board, buffer[index - 1].filename, index,
		  cookie_string);
	  printf ("● ");
	  print_title (buffer[index - 1].title);
	  printf ("</a>");
	  printf ("<td class=body%d align=right>%d\n", i % 2 + 1,
		  buffer[index - 1].accessed[0]);
	  i++;
	}

      printf ("</table>\n");
    }

  free (buffer);
  printf ("<hr>\n");

  printf ("<table class=foot>\n");
  print_go_bbsman ();

  for (index = 0; index < SectNum; index++)
    {
      if (strchr (CateInSect[index], category) != NULL)
	{
	  printf ("<th class=foot><a href=%s/bbsboa?sec=%d%s>%s類</a>\n",
		  BBSCGI, index, cookie_string, SectNames[index][0]);
	  break;
	}
    }

  sprintf (buf, "%s/vote/%s/notes", BBSHOME, board);
  if (dashf (buf))
    printf
      ("<th class=foot><a href=\"%s/bbsnot?board=%s&num=%d%s\">備忘錄</a>\n",
       BBSCGI, board, total, cookie_string);

  sprintf (buf, "%s/0Announce/.Search", BBSHOME);
  if ((fn = fopen (buf, "r")) != NULL)
    {
      while (fgets (buf, 255, fn))
	{
	  ptr = strstr (buf, ":");
	  if (ptr)
	    {
	      *ptr = '\0';
	      ptr = strtok (ptr + 1, " \n\r\t");
	      strncpy (path, ptr, STRLEN);
	      if (strncmp (buf, board, STRLEN) == 0)
		{
		  CheckPerm = 1;
		  break;
		}
	    }
	}
      (void) fclose (fn);
    }


  if (CheckPerm == 1)
    {
      sprintf (buf, "%s/0Announce/%s/.Names", BBSHOME, path);
      if ((fn = fopen (buf, "r")) != NULL)
	{
	  while (fgets (buf, 255, fn))
	    {
	      if ((strstr (buf, "(BM: BMS)") != NULL
		   && !(cookie.level & PERM_BOARDS))
		  || (strstr (buf, "(BM: SYSOPS)") != NULL
		      && !(cookie.level & PERM_SYSOP))
		  || strstr (buf, "(BM: SECRET)") != NULL)
		{
		  CheckPerm = 0;
		  break;
		}
	    }
	  (void) fclose (fn);
	}
    }

  if (CheckPerm == 1)
    printf ("<th class=foot><a href=%s/bbs0an?path=/%s%s>精華區</a>\n",
	    BBSCGI, path, cookie_string);

  printf ("<th class=foot><a href=%s/bbsdoc?board=%s%s>普通模式</a>\n",
	  BBSCGI, board, cookie_string);

  if (ch != 'A' && ch != 'T' && ch != 'W' && ch != 'C')
    {
      if (start > 1)
	printf
	  ("<th class=foot><a href=\"%s/bbstop?board=%s&go=U&to=%d%s\">上一頁</a>\n",
	   BBSCGI, board, start, cookie_string);
      if (end < total)
	printf
	  ("<th class=foot><a href=\"%s/bbstop?board=%s&go=D&to=%d%s\">下一頁</a>\n",
	   BBSCGI, board, end, cookie_string);
      if (total > 20)
	printf
	  ("<th class=foot><a href=\"%s/bbstop?board=%s&go=A%s\">全部</a>\n",
	   BBSCGI, board, cookie_string);
    }
  if (strcmp (cookie.id, "guest") != 0)
    printf
      ("<th class=foot><a href=%s/bbspst?board=%s%s>發表文章</a>\n",
       BBSCGI, board, cookie_string);
  print_go_loginout ();
  printf ("</table>\n");

  printf ("<input type=hidden name=board value=%s>\n", board);
  printf ("<select name=go>\n");
  printf ("<option value=T>搜索標題\n");
  printf ("<option value=W>搜索作者\n");
  printf ("<option value=C>搜索文章內容\n");
  printf ("<option value=S>跳轉到編號\n");
  printf ("<option value=B>跳轉到討論區\n");
  printf ("</select>\n");

  printf ("<input type=text name=to size=20 maxlength=40>\n");
  printf ("</form>\n");
  cgi_quit ();
  printf ("</center>\n");
  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
