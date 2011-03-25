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
$Id: misc.c,v 1.14 2001/02/23 21:29:01 Only Exp $
*/

#include "bbs2www.h"

struct UTMPFILE *utmpshm;

void
lockfile (int fd, int lock)
{
#ifdef HAVE_FLOCK
  if (lock == 1)
    {
      while (flock (fd, LOCK_EX))
	sleep (1);
    }
  else
    {
      while (flock (fd, LOCK_UN))
	sleep (1);
    }
#else
  struct flock arg;

  if (lock == 1)
    arg.l_type = F_WRLCK;
  else
    arg.l_type = F_UNLCK;

  arg.l_whence = arg.l_start = arg.l_len = 0;
  while (fcntl (fd, F_SETLK, &arg) == -1)
    sleep (1);
#endif
}

int
dashf (char *fname)
{
  struct stat st;
  return (stat (fname, &st) == 0 && S_ISREG (st.st_mode));
}

int
dashd (char *fname)
{
  struct stat st;
  return (stat (fname, &st) == 0 && S_ISDIR (st.st_mode));
}

void
uppercase (char *word)
{
  int i;
  char diff = 'a' - 'A';

  for (i = 0; word[i] != '\0'; i++)
    if ((word[i] >= 'a') && (word[i] <= 'z'))
      word[i] -= diff;
}

char
last_char (char *str)
{
  int len;

  if (str == NULL || *str == '\0')
    return '\0';
  else
    {
      len = strlen (str);
      return str[len - 1];
    }
}

void
print_title (char *title)
{
  int i;

  for (i = 0; title[i] != '\0'; i++)
    if (title[i] == '<')
      printf ("&lt;");
    else if (title[i] == '>')
      printf ("&gt;");
    else
      printf ("%c", title[i]);
}

void
print_user (char *username)
{
  char *ptr, *user, buf[STRLEN];

  if (username == NULL || *username == '\0')
    {
      printf (" ");
      return;
    }
  else
    {
      strncpy (buf, username, STRLEN - 1);
      user = buf;
    }

  while (*user == ' ')
    user++;

  if (*user == '\0')
    {
      printf (" ");
      return;
    }

  while ((ptr = strchr (user, ' ')) != NULL)
    {
      *ptr = '\0';
      if (last_char (user) != '.')
	printf ("<a href=\"%s/bbsqry?name=%s%s\">%s</a> ", BBSCGI, user,
		cookie_string, user);
      else
	printf ("%s ", user);
      user = ptr + 1;
    }
  if (user[0] != '\0')
    {
      if (last_char (user) != '.')
	printf ("<a href=\"%s/bbsqry?name=%s%s\">%s</a>", BBSCGI, user,
		cookie_string, user);
      else
	printf ("%s", user);
    }
}

int
quote_string (char *buf)
{
  if (strstr (buf, "的大作中提到: 】") != NULL
      || strstr (buf, "□ 引用發信人:") != NULL
      || strstr (buf, "==>[作者]:") != NULL
      || (strstr (buf, "==>") != NULL && strstr (buf, ") 提到:") != NULL)
      || (strstr (buf, "【 在") != NULL
	  && strstr (buf, "的來信中提到: 】") != NULL)
      || (strstr (buf, "==>") != NULL && strstr (buf, "] 文中提到:") != NULL)
      || (strstr (buf, "==> 在") != NULL
	  && strstr (buf, "的文章中提到:") != NULL)
      || (strstr (buf, "==> 於") != NULL
	  && strstr (buf, "文中述及:") != NULL)
      || (strstr (buf, "※ 引述") != NULL
	  && strstr (buf, "之銘言：") != NULL))
    return 1;
  else
    return 0;
}

int
countexp (struct userec *udata)
{
  int exp;

  if (strcmp (udata->userid, "guest") == 0)
    return -9999;
  exp = udata->numposts +
    udata->numlogins / 5 +
    (time (0) - udata->firstlogin) / 86400 + udata->stay / 3600;
  return exp > 0 ? exp : 0;
}

int
countperf (struct userec *udata)
{
  int perf;
  int reg_days;

  if (strcmp (udata->userid, "guest") == 0)
    return -9999;
  reg_days = (time (0) - udata->firstlogin) / 86400 + 1;

  if (reg_days <= 0)
    return 0;

  perf = ((float) udata->numposts / (float) udata->numlogins +
	  (float) udata->numlogins / (float) reg_days) * 10;
  return perf > 0 ? perf : 0;
}

int
compute_user_value (struct userec *urec)
{
  int value;

  /* if (urec) has XEMPT permission, don't kick it */
  if ((urec->userlevel & PERM_XEMPT) || strcmp (urec->userid, "guest") == 0)
    return 999;
  value = (time (0) - urec->lastlogin) / 60;	/* min */
  /* new user should register in 30 mins */
  if (strcmp (urec->userid, "new") == 0)
    {
      return (30 - value) * 60;
    }
  if (urec->numlogins <= 3)
    return (15 * 1440 - value) / 1440;
  if (!(urec->userlevel & PERM_LOGINOK))
    return (30 * 1440 - value) / 1440;
  if (urec->stay > 1000000)
    return (365 * 1440 - value) / 1440;
  return (120 * 1440 - value) / 1440;
}

char *
cexp (int exp)
{
  int expbase = 0;

  if (exp == -9999)
    return "沒等級";
  else if (exp <= 100 + expbase)
    return "新手上路";
  else if (exp > 100 + expbase && exp <= 450 + expbase)
    return "一般站友";
  else if (exp > 450 + expbase && exp <= 850 + expbase)
    return "中級站友";
  else if (exp > 850 + expbase && exp <= 1500 + expbase)
    return "高級站友";
  else if (exp > 1500 + expbase && exp <= 2500 + expbase)
    return "老站友";
  else if (exp > 2500 + expbase && exp <= 3000 + expbase)
    return "長老級";
  else if (exp > 3000 + expbase && exp <= 5000 + expbase)
    return "本站元老";
  else				/* (exp > 5000 + expbase) */
    return "幵國大老";
}

char *
cperf (int perf)
{

  if (perf == -9999)
    return "沒等級";
  else if (perf <= 5)
    return "赶快加油";
  else if (perf > 5 && perf <= 12)
    return "努力中";
  else if (perf > 12 && perf <= 35)
    return "還不錯";
  else if (perf > 35 && perf <= 50)
    return "很好";
  else if (perf > 50 && perf <= 90)
    return "优等生";
  else if (perf > 90 && perf <= 140)
    return "太优秀了";
  else if (perf > 140 && perf <= 200)
    return "本站支柱";
  else				/* (perf > 200) */
    return "神～～";
}

int
seek_in_file (char *filename, char *seekstr)
{
  FILE *fp;
  char buf[STRLEN];
  char *namep;

  sprintf (buf, "%s/%s", BBSHOME, filename);
  if ((fp = fopen (buf, "r")) == NULL)
    return 0;
  while (fgets (buf, STRLEN, fp) != NULL)
    {
      namep = (char *) strtok (buf, ": \n\r\t");
      if (namep != NULL && strcasecmp (namep, seekstr) == 0)
	{
	  fclose (fp);
	  return 1;
	}
    }
  fclose (fp);
  return 0;
}

void
print_go_bbsman ()
{
  printf ("<th class=foot><a href=\"%s/bbsman?%s\">主選單</a></th>",
	  BBSCGI, cookie_string);
}

void
print_go_bbssec ()
{
  printf ("<th class=foot><a href=\"%s/bbssec?%s\">分類討論區</a></th>",
	  BBSCGI, cookie_string);
}

void
print_go_bbsall ()
{
  printf ("<th class=foot><a href=\"%s/bbsall?%s\">全部討論區</a></th>",
	  BBSCGI, cookie_string);
}

void
print_go_bbs0an ()
{
  printf ("<th class=foot><a href=\"%s/bbs0an?%s\">精華公布欄</a></th>",
	  BBSCGI, cookie_string);
}

void
print_go_bbsmil ()
{
  if (strcmp (cookie.id, "guest") != 0)
    printf ("<th class=foot><a href=\"%s/bbsmil?%s\">處理郵件</a></th>",
	    BBSCGI, cookie_string);
}

void
print_go_loginout ()
{
  if (strcmp (cookie.id, "guest") == 0)
    printf ("<th class=foot><a href=\"%s/bbslog\">登錄</a></th>", BBSCGI);
  else
    printf ("<th class=foot><a href=\"%s/bbsout?%s\">退出</a></th>",
	    BBSCGI, cookie_string);
}

void
resolve_utmp ()
{
  int shmid;

  shmid = shmget (UTMP_SHMKEY, sizeof (struct UTMPFILE), 0);
  if (shmid < 0)
    {
      shmid =
	shmget (UTMP_SHMKEY, sizeof (struct UTMPFILE), IPC_CREAT | 0640);
      if (shmid < 0)
	show_error
	  ("Error creating UTMP share memory! Please report to SYSOP of this BBS.\n");
      utmpshm = (void *) shmat (shmid, NULL, 0);
      if (utmpshm == (void *) -1)
	show_error
	  ("Error attaching to UTMP share memory! Please report to SYSOP of this BBS.\n");
      memset (utmpshm, 0, sizeof (struct UTMPFILE));
    }
  else
    {
      utmpshm = (void *) shmat (shmid, NULL, 0);
      if (utmpshm == (void *) -1)
	show_error
	  ("Error attaching to UTMP share memory! Please report to SYSOP of this BBS.\n");
    }
}
