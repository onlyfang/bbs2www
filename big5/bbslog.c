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
$Id: bbslog.c,v 1.2 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

static char *userid, *passwd;

static void
print_table (char *information)
{

  cgi_html_head ();
  printf ("<title>%s用戶登錄</title>\n", BBSID);
  printf ("<body><center>\n");
  printf ("<form method=post action=\"%s/bbslog\">\n", BBSCGI);
  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title align=center>%s -- 用戶登錄</th>\n", BBSNAME);
  printf ("</table>\n");
  printf ("<hr>\n");

  printf ("<table class=post>\n");
  printf ("<tr><td class=post colspan=2 align=center>");
  if (information)
    printf ("%s\n", information);
  else
    printf ("請輸入用戶名和密碼\n");

  printf ("<tr><td class=post align=right>您的代號:\n");
  printf ("<td class=post align=left>");
  printf ("<input type=text name=userid size=%d", IDLEN);
  printf (" maxlength=%d value=\"%s\">\n", IDLEN, userid);

  printf ("<tr><td class=post align=right>您的密碼:\n");
  printf ("<td class=post align=left>");
  printf ("<input type=password name=passwd size=14 maxlength=%d", PASSLEN);
  printf (" value=\"%s\">\n", passwd);

  printf ("</table>\n<hr>\n");
  printf ("<input type=submit value=\"登錄\">\n");
  printf ("<table class=foot>\n");
  printf ("<th class=foot><a href=\"%s/bbsreg\">新用戶注冊</a></th>", BBSCGI);
  printf ("<th class=foot><a href=\"%s/bbsman?id=guest\">游客參觀</a></th>",
	  BBSCGI);
  printf ("</table>\n");
  printf ("</form>\n");
  printf ("</center></body>\n</html>\n");

  cgi_quit ();
  exit (0);
}

int
main ()
{
  FILE *fp;
  char buf[256], filename[STRLEN];
  int fh, found, index, status, total;
  struct userec record;
  struct stat st;
  time_t now;
  struct user_info uinfo;


  cgi_init ();

  userid = cgi_get ("userid");
  passwd = cgi_get ("passwd");
  status = atoi (cgi_get ("status"));

  if (cgi_num_entries == 0)
    print_table (NULL);

  if (status == 1)
    print_table ("用戶名或密碼錯誤! 請重新輸入");
  else if (status == 3)
    print_table ("您尚未登錄! 請登錄后再繼續");
  else if (status == 4)
    print_table ("超時退出! 請重新登錄");

  if (strcasecmp (userid, "guest") == 0)
    {
      printf ("Location: http://%s%s/bbsman\n\n", BBSHOST, BBSCGI);
      cgi_quit ();
      exit (1);
    }

  sprintf (filename, "%s/.PASSWDS", BBSHOME);
  if ((fh = open (filename, O_RDWR)) == -1)
    show_error ("Can not open .PASSWDS file!");

  (void) fstat (fh, &st);
  total = st.st_size / sizeof (record);

  found = 0;
  for (index = 0; index < total; index++)
    {
      (void) read (fh, &record, sizeof (record));
      if (strcasecmp (userid, record.userid) == 0)
	{
	  strcpy (userid, record.userid);
	  if (checkpasswd (record.passwd, passwd))
	    found = 1;
	  break;
	}
    }

  if (!found)
    {
      (void) close (fh);
      print_table ("用戶名或密碼錯誤! 請重新輸入");
    }

  record.numlogins++;
  now = time (NULL);
  record.lastlogin = now;
  strncpy (record.lasthost, getenv ("REMOTE_ADDR"), 15);
  record.lasthost[15] = '\0';
  if (CHECK_PERM && !PERM (PERM_LOGINOK) && PERM (PERM_BASIC))
    {
#ifdef FB25
      if ((strchr (record.termtype + 16, '@')) != NULL)
	record.userlevel |= PERM_DEFAULT;
#else
      sprintf (filename, "%s/home/%c/%s/register", BBSHOME,
	       toupper (userid[0]), userid);
      if ((fp = fopen (filename, "r")) != NULL)
	{
	  fgets (buf, STRLEN, fp);
	  (void) fclose (fp);
	  if (strstr (buf, "usernum"))
	    record.userlevel |= PERM_DEFAULT;
	}
#endif
    }
  lockfile (fh, 1);
  (void) lseek (fh, (off_t) (-1 * sizeof (record)), SEEK_CUR);
  (void) write (fh, &record, sizeof (record));
  lockfile (fh, 0);
  (void) close (fh);

  memset (&uinfo, 0, sizeof (uinfo));
  uinfo.active = 1;
  uinfo.pid = getpid ();
  uinfo.uid = index + 1;

  uinfo.mode = WWW;
  strncpy (uinfo.from, getenv ("REMOTE_ADDR"), 59);
  uinfo.from[59] = '\0';
#ifndef BBSD
  strcpy (uinfo.tty, "NoTTY");
#else
  uinfo.idle_time = time (NULL);
#endif
  strcpy (uinfo.userid, record.userid);
  strcpy (uinfo.username, record.username);
  strcpy (uinfo.realname, record.realname);

  sprintf (filename, "%s/home/%c/%s/.wwwlogin", BBSHOME, toupper (userid[0]),
	   userid);
  if ((fh = open (filename, O_WRONLY | O_CREAT, 0644)) == -1)
    {
      show_error ("Internel error. Can not login you in. Report to SYSOP");
    }

  write (fh, &uinfo, sizeof (uinfo));

  (void) close (fh);

  strcpy (cookie.id, record.userid);
  cookie.level = record.userlevel ^ uinfo.pid;
  sprintf (buf, "%d%s", cookie.level | uinfo.uid, uinfo.realname);
  buf[PASSLEN - 1] = '\0';
  strcpy (cookie.code, genpasswd (buf));

  printf ("Location: http://%s%s/bbsman?id=%s&code=%s&level=%d\n\n", BBSHOST,
	  BBSCGI, cookie.id, cookie.code, cookie.level);

  cgi_quit ();
  return (0);
}
