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
$Id: bbssdm.c,v 1.3 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

static char *
autocr (char *post)
{
  char *buf, *p, *q;
  int ln_len = 0, inCC = 0, postlen;

  postlen = strlen (post);
  buf = malloc (postlen + postlen / 80 + 1);
  if (buf == NULL)
    show_error ("Out of memory!");
  p = post;
  q = buf;

  while (*p)
    {
      if (ln_len == 0 && *p == ':' && *(p + 1) == ' ')
	{
	  while (*p != '\0' && *p != '\n')
	    *q++ = *p++;
	  if (*p == '\n')
	    *q++ = *p++;
	}
      else
	{
	  if (*p == '\r' && *(p + 1) == '\n')
	    p++;

	  if (*p == '\n')
	    {
	      *q++ = *p++;
	      ln_len = inCC = 0;
	    }
	  else
	    {
	      if (ln_len >= 80 && !inCC)
		{
		  *q++ = '\n';
		  ln_len = 0;
		}
	      if (*p & 0x80)
		inCC = !inCC;
	      *q++ = *p++;
	      ln_len++;
	    }
	}
    }
  *q = '\0';
  return (buf);
}


int
main ()
{
  FILE *fp, *fidx, *fout;
  char *buf, *to, article[STRLEN], buf1[256], *buf2,
    filename[STRLEN], *title, tmpsig[MAXSIGLINES][256], signfile[STRLEN];
  int i, fh, found = 0, sign, valid_ln = 0;
  struct fileheader header;
  struct userec record;
  struct user_info *uin;
  time_t now;

  cgi_init ();
  login_check ();
  if (strcmp (cookie.id, "guest") == 0)
    {
      cgi_quit ();
      printf ("Location: http://%s%s/bbslog?status=3\n\n", BBSHOST, BBSCGI);
      exit (1);
    }

  to = cgi_get ("to");
  title = cgi_get ("title");
  sign = atoi (cgi_get ("signature"));

  buf = cgi_get ("text");

  if (title[0] == '\0')
    show_error ("邮件没有标题,不能发送!");

  if (buf[0] == '\0')
    show_error ("空邮件,不能发送!");

  found = 0;
  for (i = 0; i < USHM_SIZE; i++)
    {
      uin = &(utmpshm->uinfo[i]);
      if (uin->active && uin->mode == WWW
	  && strcmp (uin->userid, cookie.id) == 0)
	{
	  found = 1;
	  break;
	}
    }
  if (found == 0)
    {
      printf ("Location: http://%s%s/bbslogin?status=3\n\n", BBSHOST, BBSCGI);
      exit (1);
    }


  if (strchr (to, '@'))
    {
      if (ALLOW_INTERNET_EMAIL != 1)
	show_error ("Internet Email not allowed in this BBS");

      sprintf (filename, "%s -f  %s.bbs@%s %s", SEND_MAIL,
	       cookie.id, BBSHOST, to);
      fout = popen (filename, "w");
      if (fout == NULL)
	show_error ("Error happened when sending your mail.");

      fprintf (fout, "Return-Path: %s.bbs@%s\n", cookie.id, BBSHOST);
      fprintf (fout, "Reply-To: %s.bbs@%s\n", cookie.id, BBSHOST);
      fprintf (fout, "From: %s.bbs@%s\n", cookie.id, BBSHOST);
      fprintf (fout, "To: %s\n", to);
      fprintf (fout, "Subject: %s\n", title);
      fprintf (fout, "X-Disclaimer: %s 对本信内容恕不负责。\n", BBSNAME);
      fprintf (fout, "X-Forwarded-By: %s (%s)\n\n", cookie.id, uin->username);
    }
  else
    {
      found = 0;
      if (to[0] != '\0')
	{
	  sprintf (filename, "%s/.PASSWDS", BBSHOME);
	  if ((fh = open (filename, O_RDONLY)) == -1)
	    show_error ("Can not open .PASSWD file!");
	  while (read (fh, &record, sizeof (record)) > 0)
	    if (strcasecmp (to, record.userid) == 0)
	      {
		found = 1;
		break;
	      }
	  (void) close (fh);
	}

      if (found == 0)
	show_error ("本站不存在该用户");
      sprintf (filename, "%s/mail/%c/%s", BBSHOME,
	       toupper (record.userid[0]), record.userid);

      if (!dashd (filename))
	(void) mkdir (filename, 0755);

      strcat (filename, "/.DIR");
      if ((fidx = fopen (filename, "a")) == NULL)
	show_error ("Write to .DIR file failed!");
      now = time (NULL);
      sprintf (filename, "M.%ld.A", now);
      sprintf (article, "%s/mail/%c/%s/%s", BBSHOME,
	       toupper (record.userid[0]), record.userid, filename);

      fout = fopen (article, "w+");
      if (fout == NULL)
	{
	  (void) fclose (fidx);
	  show_error ("Write to mail dir failed!");
	}

      bzero ((void *) &header, sizeof (header));
      strcpy (header.filename, filename);
      strncpy (header.owner, cookie.id, IDLEN);
      strncpy (header.title, title, STRLEN);
      (void) fwrite (&header, sizeof (header), 1, fidx);
      (void) fclose (fidx);
      sprintf (buf1,
	       "寄信人: %s (%s)\n标  题: %s\n发信站: %s (%.24s)\n来  源: BBS2WWW\n\n",
	       cookie.id, uin->username, title, BBSNAME, ctime (&now));
      (void) fwrite (buf1, sizeof (char), strlen (buf1), fout);
    }

  buf2 = autocr (buf);
  (void) fwrite (buf2, sizeof (char), strlen (buf2), fout);
  free (buf2);
  (void) fwrite ("\n\n--\n", sizeof (char), 5, fout);
  if (sign > 0)
    {
      sprintf (signfile, "%s/home/%c/%s/signatures", BBSHOME,
	       toupper (record.userid[0]), record.userid);
      if ((fp = fopen (signfile, "r")) != NULL)
	{
	  for (i = 1; i <= (sign - 1) * MAXSIGLINES; i++)
	    if (!fgets (buf1, 255, fp))
	      break;
	  for (i = 1; i <= MAXSIGLINES; i++)
	    {
	      if (fgets (buf1, 255, fp))
		{
		  if (buf1[0] != '\n')
		    valid_ln = i;
		  strcpy (tmpsig[i - 1], buf1);
		}
	      else
		break;
	    }
	  (void) fclose (fp);
	  for (i = 1; i <= valid_ln; i++)

	    (void) fwrite (tmpsig[i - 1], sizeof (char),
			   strlen (tmpsig[i - 1]), fout);
	}
    }

  if (HIDE_IP == 1)
    sprintf (buf1, "※ 来源:．%s %s. [FROM: WWW]\n", BBSNAME, BBSHOST);
  else
    sprintf (buf1,
	     "※ 来源:．%sWWW %s. [FROM: %.20s]\n",
	     BBSNAME, BBSHOST, getenv ("REMOTE_ADDR"));
  (void) fwrite (buf1, sizeof (char), strlen (buf1), fout);

  if (strchr (to, '@'))
    (void) pclose (fout);
  else
    (void) fclose (fout);

  printf ("Location: http://%s%s/bbsmil?%s\n\n", BBSHOST,
	  BBSCGI, cookie_string);
  cgi_quit ();
  return (0);
}
