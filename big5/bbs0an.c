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
$Id: bbs0an.c,v 1.13 2001/02/23 21:28:59 Only Exp $
*/

#include "bbs2www.h"

int
main ()
{
  FILE *inf;
  char filename[STRLEN], *ptr, *path, buf[256], title[STRLEN],
    fname[STRLEN], *user;
  int index = 0, num = 0, pos;
  struct stat st;
  struct tm *pt;

  cgi_init ();
  login_check ();
  cgi_html_head ();
  printf ("<title>%s精華區</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  path = cgi_get ("path");

  sprintf (filename, "%s/0Announce%s/.Names", BBSHOME, path);

  if (!(inf = fopen (filename, "r")))
    {
      sprintf (buf, "Can not open file %s!\n", filename);
      show_error (buf);
    }

  while (fgets (buf, STRLEN, inf))
    {
      if (strncmp (buf, "Numb=", 5) == 0)
	{
	  num = 1;
	  break;
	}
    }
  rewind (inf);

  while (fgets (buf, STRLEN, inf))
    {
      if ((ptr = strchr (buf, '\n')) != NULL)
	*ptr = '\0';
      if (strncmp (buf, "# Title=", 8) == 0)
	{
	  if (strstr (buf, "(BM: BMS)") != NULL
	      || strstr (buf, "(BM: SYSOPS)") != NULL
	      || strstr (buf, "(BM: SECRET)") != NULL)
	    {
	      printf ("Error input!");
	      exit (0);
	    }
	  if ((ptr = strstr (buf, "(BM: ")) != NULL)
	    *ptr = '\0';
	  ptr = buf + strlen (buf) - 1;
	  while (*ptr == ' ')
	    *ptr-- = '\0';

	  printf ("<table class=title width=90%%><tr>");
	  printf ("<th class=title align=left width=33%%>精華區</th>\n");
	  printf ("<th class=title align=center width=33%%>%s</th>\n",
		  BBSNAME);
	  if (path[0] != '\0')
	    {
	      printf ("<th class=title align=right width=34%%>%s</th>\n",
		      buf + 8);
	    }
	  else
	    printf
	      ("<th class=title align=right width=34%%>精華公布欄</th>\n");
	  printf ("</table>\n");

	  printf ("<hr>\n");
	  if (num == 0)
	    {
	      printf ("<p><< 目前沒有文章 >>\n</p>");
	      break;
	    }
	  else
	    {
	      printf ("<table class=body>\n");
	      printf ("<tr>");
	      printf
		("<th class=body>編號<th class=body>類別<th class=body>");
	      printf ("標題<th class=body>整理<th class=body>編輯日期\n");
	    }
	}
      else if (strncmp (buf, "Name=", 5) == 0)
	{
	  memset (title, 0, STRLEN);
	  strncpy (title, buf + 5, sizeof (title));
	  if (title[39] != '\0')
	    {
	      if (title[38] == '(')
		user = title + 43;
	      else
		user = title + 39;
	      pos = strlen (user) - 1;
	      for (pos = strlen (user) - 1; pos >= 0; pos--)
		{
		  if (user[pos] == ')' || user[pos] == ' ')
		    user[pos] = '\0';
		  else
		    break;
		}
	    }
	  else
	    user = "\0";
	}
      else if (strncmp (buf, "Path=", 5) == 0)
	{
	  if (strncmp (buf, "Path=~/", 7) == 0)
	    strncpy (fname, buf + 7, sizeof (fname));
	  else
	    strncpy (fname, buf + 5, sizeof (fname));
	  if (!strstr (title, "(BM: BMS)") && !strstr (title, "(BM: SYSOPS)"))
	    {
	      pos = 37;
	      while (title[pos] == ' ')
		title[pos--] = '\0';
	      index++;
	      printf ("<tr><td class=body%d>%d<td class=body%d>",
		      (index - 1) % 2 + 1, index, (index - 1) % 2 + 1);
	      sprintf (filename, "%s/0Announce%s/%s", BBSHOME, path, fname);
	      if (stat (filename, &st) != 0)
		{
		  printf ("錯誤<td class=body%d>", (index - 1) % 2 + 1);
		  print_title (title);
		  printf ("<td class=body%d> \n", (index - 1) % 2 + 1);
		  if (user[0] != '\0')
		    printf ("%s", user);
		  else
		    printf (" ");
		  printf ("<td class=body%d> <td class=body%d> \n",
			  (index - 1) % 2 + 1, (index - 1) % 2 + 1);
		}
	      else
		{
		  if (S_ISREG (st.st_mode))
		    {
		      printf ("文件");
		      printf ("<td class=body%d>", (index - 1) % 2 + 1);
		      printf ("<a href=\"%s/bbsanc?path=%s/%s%s\">",
			      BBSCGI, path, fname, cookie_string);
		    }
		  else if (S_ISDIR (st.st_mode))
		    {
		      printf ("目錄");
		      printf ("<td class=body%d>", (index - 1) % 2 + 1);
		      printf ("<a href=\"%s/bbs0an?path=%s/%s%s\">",
			      BBSCGI, path, fname, cookie_string);
		    }
		  print_title (title);
		  printf ("</a><td class=body%d>", (index - 1) % 2 + 1);
		  print_user (user);

		  pt = localtime (&(st.st_mtime));
		  pt->tm_year += 1900;
		  printf ("<td class=body%d>%04d.%02d.%02d\n",
			  (index - 1) % 2 + 1,
			  pt->tm_year, pt->tm_mon + 1, pt->tm_mday);
		}
	    }
	}
    }

  cgi_quit ();
  (void) fclose (inf);
  printf ("</table>\n");

  printf ("<hr>\n");

  printf ("<table class=foot>\n");

  print_go_bbsman ();
  print_go_bbssec ();
  print_go_bbsall ();

  if (strlen (path) > 1)
    for (index = strlen (path) - 2; index >= 0; index--)
      {
	if (path[index] == '/')
	  {
	    path[index] = '\0';
	    printf
	      ("<th class=foot><a href=\"%s/bbs0an?path=%s%s\">上層目錄</a>",
	       BBSCGI, path, cookie_string);
	    break;
	  }
      }

  print_go_loginout ();
  printf ("</table>\n");

  printf ("</center>\n");
  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
