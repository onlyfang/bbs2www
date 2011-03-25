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
$Id: bbsqry.c,v 1.10 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

static int
check_query_mail (char *qry_mail_dir)
{
  struct fileheader fh;
  struct stat st;
  int fd;
  register int offset;
  register long numfiles;
  unsigned char ch;

  offset = (int) ((char *) &(fh.accessed[0]) - (char *) &(fh));
  if ((fd = open (qry_mail_dir, O_RDONLY)) < 0)
    return 0;
  (void) fstat (fd, &st);
  numfiles = (long int) st.st_size;
  numfiles = numfiles / sizeof (struct fileheader);
  if (numfiles <= 0)
    {
      (void) close (fd);
      return 0;
    }
  (void) lseek (fd, (off_t) (st.st_size - (sizeof (fh) - offset)), SEEK_SET);
  (void) read (fd, &ch, 1);
  if (!(ch & FILE_READ))
    {
      (void) close (fd);
      return 1;
    }
  (void) close (fd);
  return 0;
}

int
main ()
{
  FILE *fp;
  char filename[STRLEN], buf1[STRLEN], *newline, *username;
  int i, exp, num, perf, fh, found = 0;
  struct user_info *uin;
  struct userec record;


  cgi_init ();
  login_check ();
  cgi_html_head ();
  printf ("<title>%s查询网友</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title align=center>%s -- 查询网友</th>\n", BBSNAME);
  printf ("</table>\n");
  printf ("<hr>\n");

  username = cgi_get ("name");

  if (username[0] != '\0')
    {
      sprintf (filename, "%s/.PASSWDS", BBSHOME);
      if ((fh = open (filename, O_RDONLY)) == -1)
	show_error ("Can not open .PASSWD file!");

      while (read (fh, &record, sizeof (record)) > 0)
	if (strcasecmp (username, record.userid) == 0)
	  {
	    found = 1;
	    break;
	  }
      (void) close (fh);
    }

  if (found == 0)
    {

      if (username[0] != '\0')
	printf ("<p>用户名 <B>%s</B> 不存在!", username);
      else
	printf ("<p>请输入用户名:\n");

      printf ("<form action=\"%s/bbsqry\">", BBSCGI);
      printf ("<input type=submit value=\"查询用户\">");
      printf (" <input type=text name=name size=%d maxlength=%d>", IDLEN,
	      IDLEN);
      printf ("<input type=hidden name=id value=%s>\n", cookie.id);
      printf ("<input type=hidden name=code value=%s>\n", cookie.code);
      printf ("<input type=hidden name=level value=%s>\n", cgi_get ("level"));
      printf ("</form></p>");
    }
  else
    {
      printf ("<table class=doc>\n");
      printf ("<tr><td class=doc><pre>");

      printf
	("<font class=col37>%s</font> (<font class=col33>%s</font>) 共上站 <font class=col32>%u</font> 次，发表过 <font class=col32>%u</font> 篇文章\n",
	 record.userid, record.username, record.numlogins, record.numposts);
      strcpy (buf1, ctime (&(record.lastlogin)));
      if ((newline = strchr (buf1, '\n')) != NULL)
	*newline = '\0';
      printf
	("上次在 [<font class=col32>%s</font>] 从 [<font class=col32>%s</font>] 到本站一游。\n",
	 buf1, (record.lasthost[0] == '\0' ? "(不详)" : record.lasthost));

      exp = countexp (&record);
      perf = countperf (&record);
      sprintf (filename, "%s/mail/%c/%s/.DIR", BBSHOME,
	       toupper (record.userid[0]), record.userid);
      printf
	("信箱：[<blink><font class=col32>%2s</font></blink>]，经验值：[<font class=col32>%d</font>](<font class=col33>%s</font>) 表现值：[<font class=col32>%d</font>](<font class=col33>%s</font>) 生命力：[<font class=col32>%d</font>]。\n",
	 (check_query_mail (filename) == 1) ? "⊙" : "  ", exp, cexp (exp),
	 perf, cperf (perf), compute_user_value (&record));

      resolve_utmp ();
      num = 0;
      for (i = 0; i < USHM_SIZE; i++)
	{
	  uin = &(utmpshm->uinfo[i]);

	  if (strcmp (uin->userid, record.userid) == 0)
	    {
	      if (uin->active == 0 || uin->pid == 0 || uin->invisible != 0)
		continue;
	      num++;
	      if (num == 1)
		printf ("<tr><td class=doc><pre>目前在站上，状态如下：\n");
	      printf ("<b>%-14s</b> ", ModeType (uin->mode));
	      if ((num) % 5 == 0)
		printf ("<br>\n");
	    }
	}

      sprintf (filename, "%s/home/%c/%s/plans", BBSHOME,
	       toupper (record.userid[0]), record.userid);
      if ((fp = fopen (filename, "r")) == NULL)
	{
	  printf
	    ("<tr><td class=doc><font class=col36>没有个人说明档</font>\n");
	}
      else
	{
	  printf
	    ("<tr><td class=doc><font class=col36>个人说明档如下：</font>\n");
	  display_article (fp, 1);

	  (void) fclose (fp);
	}

      printf ("</pre></table>\n");
    }

  printf ("<hr>");

  printf ("<table class=foot>\n");
  print_go_bbsman ();
  print_go_bbssec ();
  print_go_bbsall ();
  print_go_loginout ();
  printf ("</table>\n");

  cgi_quit ();
  printf ("</center>\n");
  printf ("</body>\n");
  printf ("</html>");

  return (0);
}
