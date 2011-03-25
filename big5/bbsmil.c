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
$Id: bbsmil.c,v 1.3 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

int
main ()
{
  char buf[256], ch, *go, *to, *ptr, *date, type;
  int i, index, fd, number = 0, start = 0, end = 0, total = 0;
  struct fileheader *buffer;
  struct stat st;
  time_t filetime;
#ifdef COLOR_POST_DATE
  struct tm *mytm;
#endif


  cgi_init ();
  login_check ();
  if (strcmp (cookie.id, "guest") == 0)
    {
      cgi_quit ();
      printf ("Location: http://%s%s/bbslog?status=3\n\n", BBSHOST, BBSCGI);
      exit (1);
    }
  cgi_html_head ();
  printf ("<title>%sMail Operation</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  go = cgi_get ("go");
  to = cgi_get ("to");

  if (strlen (go) > 0)
    ch = go[0];
  else
    ch = 'N';

  if ((ch == 'U') || (ch == 'D'))
    number = atoi (to);

  printf ("<form action=\"%s/bbsmil\">\n", BBSCGI);

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>");
  printf ("處理郵件");
  printf ("</th>\n<th class=title width=33%% align=center>%s</th>\n",
	  BBSNAME);
  printf ("<th class=title width=34%% align=right>用戶名 [%s]</th>\n",
	  cookie.id);
  printf ("</table>\n");

  sprintf (buf, "%s/mail/%c/%s/.DIR", BBSHOME, toupper (cookie.id[0]),
	   cookie.id);
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

  printf ("<hr>\n");

  if (total == 0 && (ch == 'N' || ch == 'B'))
    {
      printf ("沒有新郵件\n");
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
	default:
	  end = total;
	  start = total - 19;
	  if (start < 1)
	    start = 1;
	}

      printf ("<table class=body>\n");
      printf
	("<tr><th class=body>序號<th class=body>狀態<th class=body>發信者");
      printf ("<th class=body>日期<th class=body>標題\n");

      strncpy (buf, to, 255);
      buf[255] = '\0';
      uppercase (buf);
      i = 0;
      for (index = start; index <= end; index++)
	{
	  type = ' ';
	  printf ("<tr><td class=body%d>%d<td class=body%d>", i % 2 + 1,
		  index, i % 2 + 1);
	  if (buffer[index - 1].accessed[0] & FILE_READ)
	    {
	      if (buffer[index - 1].accessed[0] & FILE_MARKED)
		type = 'm';
	      else
		type = ' ';
	    }
	  else
	    {
	      if (buffer[index - 1].accessed[0] & FILE_MARKED)
		type = 'M';
	      else
		type = 'N';
	    }

	  if (type != ' ' && type != 'N')
	    printf ("<font class=col31>%c</font>", type);
	  else
	    printf ("%c", type);

	  printf ("<td class=body%d>", i % 2 + 1);

	  ptr = strchr (buffer[index - 1].owner, '<');
	  if (ptr != NULL && ptr[strlen (ptr) - 1] == '>')
	    {
	      ptr[strlen (ptr) - 1] = '\0';
	      printf ("%s", ptr);
	    }
	  else
	    {
	      ptr = strchr (buffer[index - 1].owner, ' ');
	      if (ptr != NULL)
		*ptr = '\0';
	      print_user (buffer[index - 1].owner);
	    }

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
	  printf ("<a href=\"%s/bbsmlc?file=%s&num=%d%s\">",
		  BBSCGI, buffer[index - 1].filename, index, cookie_string);
	  if (strncmp ("Re:", buffer[index - 1].title, 3) != 0
	      && strncmp ("RE:", buffer[index - 1].title, 3) != 0)
	    printf ("● ");
	  print_title (buffer[index - 1].title);
	  printf ("</a>\n");
	  i++;
	}

      printf ("</table>\n");
    }

  free (buffer);
  printf ("<hr>\n");

  printf ("<table class=foot>\n");
  print_go_bbsman ();

  if (ch != 'A')
    {
      if (start > 1)
	printf
	  ("<th class=foot><a href=\"%s/bbsmil?go=U&to=%d%s\">上一頁</a>\n",
	   BBSCGI, start, cookie_string);
      if (end < total)
	printf
	  ("<th class=foot><a href=\"%s/bbsmil?go=D&to=%d%s\">下一頁</a>\n",
	   BBSCGI, end, cookie_string);
      if (total > 20)
	printf
	  ("<th class=foot><a href=\"%s/bbsmil?go=A%s\">全部</a>\n",
	   BBSCGI, cookie_string);
    }
  printf
    ("<th class=foot><a href=%s/bbspsm?%s>發郵件</a>\n",
     BBSCGI, cookie_string);

  print_go_loginout ();
  printf ("</table>\n");
  printf ("</form>\n");
  cgi_quit ();
  printf ("</center>\n");
  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
