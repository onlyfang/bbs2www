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
$Id: bbsusr.c,v 1.10 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

static struct user_info user_record[MAXACTIVE];

static void
swap_user_record (int a, int b)
{
  struct user_info c;

  memcpy (&c, &user_record[a], sizeof (struct user_info));
  memcpy (&user_record[a], &user_record[b], sizeof (struct user_info));
  memcpy (&user_record[b], &c, sizeof (struct user_info));
}

static void
sort_user_record (int left, int right)
{
  int i, last;

  if (left >= right)
    return;
  swap_user_record (left, (left + right) / 2);
  last = left;
  for (i = left + 1; i <= right; i++)
    if (strcasecmp (user_record[i].userid, user_record[left].userid) < 0)
      swap_user_record (++last, i);
  swap_user_record (left, last);
  sort_user_record (left, last - 1);
  sort_user_record (last + 1, right);
}

static char *
idle_str (uent)
     struct user_info *uent;
{
  static char hh_mm_ss[32];
#ifndef BBSD
  struct stat buf;
  char tty[128];
#endif
  time_t now, diff;
  int limit, hh, mm;

  if (uent == NULL)
    {
      strcpy (hh_mm_ss, "不详");
      return hh_mm_ss;
    }

#ifndef BBSD
  strcpy (tty, uent->tty);

  if (stat (tty, &buf) != 0
      || (strstr (tty, "tty") == NULL && strstr (tty, "pts") == NULL))
    {
      strcpy (hh_mm_ss, "不详");
      return hh_mm_ss;
    }
#endif

  now = time (0);

#ifndef BBSD
  diff = now - buf.st_atime;
#else
  diff = now - uent->idle_time;
#endif

#ifdef DOTIMEOUT
  /* the 60 * 60 * 24 * 5 is to prevent fault /dev mount from
     kicking out all users */

#ifndef FB25
  if (uent->ext_idle)
    limit = IDLE_TIMEOUT * 3;
  else
#endif
    limit = IDLE_TIMEOUT;

  if ((diff > limit) && (diff < 86400 * 5))
    kill (uent->pid, SIGHUP);
#endif

  hh = diff / 3600;
  mm = (diff / 60) % 60;

  if (hh > 0)
    sprintf (hh_mm_ss, "%d:%02d", hh, mm);
  else if (mm > 0)
    sprintf (hh_mm_ss, "%d", mm);
  else
    sprintf (hh_mm_ss, "   ");

  return hh_mm_ss;
}

int
main ()
{
  int i, num;
  struct user_info *uin;

  cgi_init ();
  login_check ();
  cgi_quit ();
  cgi_html_head ();
  printf ("<title>%s使用者列表</title>\n", BBSID);
  printf ("<body>\n");
  printf ("<center>\n");

  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title align=center>%s -- 在线用户列表</th>\n", BBSNAME);
  printf ("</table>\n");

  printf ("<hr>\n");

  resolve_utmp ();
  num = 0;
  for (i = 0; i < USHM_SIZE; i++)
    {
      uin = &(utmpshm->uinfo[i]);

      if (!uin->active || !uin->pid || uin->invisible)
	continue;
      memcpy (&user_record[num], uin, sizeof (struct user_info));
      num++;
    }

  if (num == 0)
    {
      printf ("<p>目前没有用户在站内</p>\n");
    }
  else
    {
      printf ("<table class=body>\n");
      printf ("<tr><th class=body>序号<th class=body>使用者代号");
      printf ("<th class=body>使用者昵称<th class=body>来自<th class=body>");
      printf ("动态<th class=body>时:分\n");

      sort_user_record (0, num - 1);

      for (i = 0; i < num; i++)
	{
	  printf ("<tr><td class=body%d>%d", i % 2 + 1, i + 1);
	  printf ("<td class=body%d><a href=\"%s/bbsqry?name=%s%s\">%s</a>",
		  i % 2 + 1, BBSCGI, user_record[i].userid, cookie_string,
		  user_record[i].userid);
	  printf ("<td class=body%d><a href=\"%s/bbsqry?name=%s%s\">%s</a>",
		  i % 2 + 1, BBSCGI, user_record[i].userid, cookie_string,
		  user_record[i].username);
	  printf ("<td class=body%d>%s", i % 2 + 1, user_record[i].from);
	  printf ("<td class=body%d>%s", i % 2 + 1,
		  ModeType (user_record[i].mode));
	  printf ("<td class=body%d align=right>%s", i % 2 + 1,
		  idle_str (&user_record[i]));
	}

      printf ("</table>\n");
    }


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
