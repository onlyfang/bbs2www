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
$Id: bbspst.c,v 1.13 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

#define ANONY_FLAG      0x8

int
main ()
{
  FILE *fp;
  char LineBuf[256], *board, *quser, *ptr,
    buf[256], *filename, *title, DirOfBoard[256];
  int fd, index = 0, newfile, noname, found;
  struct boardheader brdhdr;
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
  printf ("<title>%s發表文章</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  board = cgi_get ("board");
  filename = cgi_get ("file");
  if (filename[0] != '\0')
    newfile = 0;
  else
    {
      title = cgi_get ("title");
      newfile = 1;
    }

  printf ("<form method=POST action=\"%s/bbssnd\">\n", BBSCGI);

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title width=33%% align=left>發表文章</th>\n");
  printf ("<th class=title width=33%% align=center>%s</th>\n", BBSNAME);
  printf ("<th class=title width=34%% align=right>討論區 [%s]</th>\n", board);
  printf ("</table>\n");

  printf ("<hr>\n");

  found = 0;
  sprintf (DirOfBoard, "%s/.BOARDS", BBSHOME);
  if ((fd = open (DirOfBoard, O_RDONLY, 0)) == -1)
    show_error ("Can not open .BOARDS file!");

  while (read (fd, &brdhdr, sizeof (brdhdr)) == (ssize_t) sizeof (brdhdr))
    if (strcasecmp (brdhdr.filename, board) == 0)
      {
	found = 1;
	break;
      }
  (void) close (fd);
  if (found == 0)
    {
      sprintf (buf, "找不到討論區 %s!", board);
      show_error (buf);
    }

  sprintf (DirOfBoard, "%s/vote/%s/notes", BBSHOME, board);

  if ((fp = fopen (DirOfBoard, "r")) != NULL)
    {
      printf ("<table class=doc>");
      printf ("<tr><td class=doc>");

      display_article (fp, 1);

      printf ("</table><hr>\n");

      (void) fclose (fp);
    }

#ifdef FB25
  noname = seek_in_file ("etc/anonymous", board);
#else
  noname = brdhdr.flag & ANONY_FLAG;
#endif

  printf ("<input type=hidden name=board value=%s>\n", board);
  printf ("<input type=hidden name=id value=%s>\n", cookie.id);
  printf ("<input type=hidden name=code value=%s>\n", cookie.code);
  printf ("<input type=hidden name=level value=%s>\n", cgi_get ("level"));
  printf ("<table class=post>\n");
  printf
    ("<tr><td class=post>使用標題 <input type=text name=title size=40 maxlength=100 ");
  printf ("value=\"");
  if (newfile == 0)
    {
      sprintf (DirOfBoard, "%s/boards/%s/.DIR", BBSHOME, board);
      fd = open (DirOfBoard, O_RDONLY);
      if (fd == -1)
	show_error ("Can not open .DIR file!");

      while (read (fd, &DirInfo, sizeof (DirInfo)) ==
	     (ssize_t) sizeof (DirInfo))
	{
	  if (strcmp (DirInfo.filename, filename) == 0)
	    {
	      if (strncmp (DirInfo.title, "Re: ", sizeof (char) * 4) != 0)
		printf ("Re: ");
	      printf ("%s", DirInfo.title);
	      break;
	    }
	}
      (void) close (fd);
    }
  else if (title[0] != '\0')
    {
      printf ("Re: %s", title);
    }

  printf ("\"> ");
  if (noname != 0)
    {
      printf ("<input type=radio name=anonymous value=Y checked>匿名 ");
      printf ("<input type=radio name=anonymous value=N>不匿名");
      printf ("<input type=hidden name=exchange value=N>");
    }
  else
    {
      printf ("<input type=hidden name=anonymous value=N>");
      printf ("<input type=radio name=exchange value=Y checked>轉信 ");
      printf ("<input type=radio name=exchange value=N>站內信件");
    }

  printf ("<tr><td class=post>");
  printf ("使用簽名檔 ");
  printf ("<input type=radio name=signature value=1");
  if (noname != 0)
    printf (">1");
  else
    printf (" checked>1");
  printf ("<input type=radio name=signature value=2>2");
  printf ("<input type=radio name=signature value=3>3");
  printf ("<input type=radio name=signature value=0");
  if (noname != 0)
    printf (" checked>0");
  else
    printf (">0");

  printf ("<tr><td class=post>");
  printf ("<textarea name=text rows=25 cols=80 wrap=physicle>\n");

  if (newfile == 0)
    {
      sprintf (LineBuf, "%s/boards/%s/%s", BBSHOME, board, filename);

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
	      printf ("【 在 %s 的大作中提到: 】\n", quser);
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
  printf ("<tr><td class=post align=center>");
  printf ("<input type=submit value=\"發表\">");
  printf (" <input type=reset value=\"清除\">\n</table>\n");

  printf ("<hr>\n");

  printf ("</center>\n");
  printf ("</form>\n");
  cgi_quit ();
  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
