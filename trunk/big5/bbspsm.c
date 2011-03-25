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
$Id: bbspsm.c,v 1.3 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

#define ANONY_FLAG      0x8

int
main ()
{
  FILE *fp;
  char LineBuf[256], *quser, *ptr, buf[256], *filename, DirOfBoard[256];
  int fd, index = 0, newfile;
  struct fileheader DirInfo;


  cgi_init ();
  login_check ();
  if (strcmp (cookie.id, "guest") == 0)
    {
      cgi_quit ();
      printf ("Location: http://%s%s/bbslog?status=3\n\n", BBSHOST, BBSCGI);
      exit (1);
    }
  cgi_html_head ();
  printf ("<title>%s發新郵件</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  filename = cgi_get ("file");
  if (filename[0] != '\0')
    newfile = 0;
  else
    newfile = 1;

  printf ("<form method=POST action=\"%s/bbssdm\">\n", BBSCGI);

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>發新郵件</th>\n");
  printf ("<th class=title width=33%% align=center>%s</th>\n", BBSNAME);
  printf ("<th class=title width=34%% align=right>用戶名 [%s]</th>\n",
	  cookie.id);
  printf ("</table>\n");

  printf ("<hr>\n");

  printf ("<input type=hidden name=id value=%s>\n", cookie.id);
  printf ("<input type=hidden name=code value=%s>\n", cookie.code);
  printf ("<input type=hidden name=level value=%s>\n", cgi_get ("level"));
  printf ("<table class=post>\n");
  printf
    ("<tr><td class=post>收信人<td class=post><input type=text name=to size=25 maxlength=40 ");
  printf ("value=\"");
  if (newfile == 0)
    {
      sprintf (DirOfBoard, "%s/mail/%c/%s/.DIR", BBSHOME,
	       toupper (cookie.id[0]), cookie.id);

      fd = open (DirOfBoard, O_RDONLY);
      if (fd == -1)
	show_error ("Can not open .DIR file!");

      while (read (fd, &DirInfo, sizeof (DirInfo)) ==
	     (ssize_t) sizeof (DirInfo))
	{
	  if (strcmp (DirInfo.filename, filename) == 0)
	    {
	      ptr = strchr (DirInfo.owner, '<');
	      if (ptr != NULL && ptr[strlen (ptr) - 1] == '>')
		{
		  ptr[strlen (ptr) - 1] = '\0';
		  printf ("%s", ptr);
		}
	      else
		{
		  ptr = strchr (DirInfo.owner, ' ');
		  if (ptr != NULL)
		    *ptr = '\0';
		  printf ("%s", DirInfo.owner);
		}

	      break;
	    }
	}
      (void) close (fd);
    }
  printf ("\"> ");
  printf ("<td class=post>使用簽名檔");
  printf ("<input type=radio name=signature value=1");
  printf (" checked>1");
  printf ("<input type=radio name=signature value=2>2");
  printf ("<input type=radio name=signature value=3>3");
  printf ("<input type=radio name=signature value=0");
  printf (">0");

  printf
    ("<tr><td class=post>郵件標題<td class=post colspan=2><input type=text name=title size=40 maxlength=100 ");
  printf ("value=\"");
  if (newfile == 0)
    {
      if (strncmp (DirInfo.title, "Re: ", sizeof (char) * 4) != 0)
	printf ("Re: ");
      printf ("%s", DirInfo.title);
    }

  printf ("\"> ");

  printf ("<tr><td class=post colspan=3>");
  printf ("<textarea name=text rows=25 cols=80 wrap=physicle>\n");

  if (newfile == 0)
    {
      sprintf (LineBuf, "%s/mail/%c/%s/%s", BBSHOME, toupper (cookie.id[0]),
	       cookie.id, filename);

      fp = fopen (LineBuf, "r");
      if (fp == NULL)
	show_error ("Can not open required file!");

      (void) fgets (buf, 256, fp);
      if ((ptr = strrchr (buf, ')')) != NULL)
	{
	  ptr[1] = '\0';
	  if ((ptr = strchr (buf, ':')) != NULL)
	    {
	      quser = ptr + 1;
	      while (*quser == ' ')
		quser++;
	      printf ("【 在 %s 的來信中提到: 】\n", quser);
	    }
	}
      while (fgets (buf, 256, fp) != NULL)
	if (buf[0] == '\n')
	  break;
      while (fgets (buf, 256, fp) != NULL)
	{
	  if (strcmp (buf, "--\n") == 0)
	    break;
	  printf (": ");
	  index = 0;
	  while (index < 256 && buf[index] != '\0')
	    {
	      if (buf[index] != '\x1b')
		printf ("%c", buf[index]);
	      else
		while (buf[index] != 'm' && index < 256)
		  index++;
	      index++;
	    }
	}
    }
  printf ("</textarea>\n");
  printf ("<tr><td class=post align=center colspan=3>");
  printf ("<input type=submit value=\"發送\">");
  printf (" <input type=reset value=\"清除\">\n</table>\n");

  printf ("<hr>\n");

  printf ("</center>\n");
  printf ("</form>\n");
  cgi_quit ();
  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
