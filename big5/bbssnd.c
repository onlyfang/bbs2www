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
$Id: bbssnd.c,v 1.13 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

static int
junkboard (char *board)
{
#ifdef FB30
  if (strstr (JUNKBOARDS, board))
    return 1;
  else
    return 0;
#else
  return (seek_in_file ("etc/junkboards", board));
#endif
}

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
  FILE *fp, *fidx;
  char *buf, *ptr, article[STRLEN], *board, buf1[256], *buf2,
    *exch, *anonymous, filename[STRLEN], *title, *signature,
    tmpsig[MAXSIGLINES][256], signfile[STRLEN];
  int i, color, exchange = 1, fh, found = 0, noname = 0, sign, valid_ln = 0;
  struct boardheader brdhdr;
  struct fileheader header;
  struct userec record;
  struct user_info *uin;
  time_t now;
  struct
  {
    char author[IDLEN + 1];
#ifdef FB25
    char board[IDLEN + 1];
#else
    char board[18];
#endif
    char title[66];
    time_t date;
    int number;
  }
  postlog;

  cgi_init ();
  login_check ();
  if (strcmp (cookie.id, "guest") == 0)
    {
      cgi_quit ();
      printf ("Location: http://%s%s/bbslog?status=3\n\n", BBSHOST, BBSCGI);
      exit (1);
    }

  buf = cgi_get ("text");

  board = cgi_get ("board");
  exch = cgi_get ("exchange");
  if (exch[0] == 'N')
    exchange = 0;
  anonymous = cgi_get ("anonymous");
  if (anonymous[0] == 'Y')
    noname = 1;
  title = cgi_get ("title");
  signature = cgi_get ("signature");
  sign = atoi (signature);

  if (title[0] == '\0')
    show_error ("文章沒有標題,不能發表!");

  if (buf[0] == '\0')
    show_error ("空文章,不能發表!");


  sprintf (filename, "%s/boards/%s/.DIR", BBSHOME, board);
  if ((fidx = fopen (filename, "r+")) == NULL)
    if ((fidx = fopen (filename, "w+")) == NULL)
      show_error ("Write to .DIR file failed!");

  found = 0;
  sprintf (filename, "%s/.BOARDS", BBSHOME);
  if ((fh = open (filename, O_RDONLY, 0)) == -1)
    show_error ("Can not open .BOARDS file!");

  while (read (fh, &brdhdr, sizeof (brdhdr)) == sizeof (brdhdr))
    if (strcasecmp (brdhdr.filename, board) == 0)
      {
	found = 1;
	break;
      }
  (void) close (fh);
  if (found == 0)
    {
      sprintf (buf1, "找不到討論區 %s!", board);
      show_error (buf1);
    }

  if (!(cookie.level & PERM_SYSOP))
    {
      if (!(cookie.level & PERM_POST)
	  || (brdhdr.level > 0 && !(cookie.level & brdhdr.level))
	  || (cookie.level & PERM_DENYPOST))
	{
	  sprintf (buf1, "抱歉,妳沒有在%s討論區發表文章的權限!", board);
	  show_error (buf1);
	}

      sprintf (filename, "boards/%s/deny_users", board);
      if (seek_in_file (filename, cookie.id))
	{
	  sprintf (buf1, "抱歉,妳沒有在%s討論區發表文章的權限!", board);
	  show_error (buf1);
	}
    }
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
  sprintf (filename, "%s/.PASSWDS", BBSHOME);
  if ((fh = open (filename, O_RDWR)) == -1)
    show_error ("Can not open .PASSWDS file!");
  lockfile (fh, 1);
  (void) lseek (fh, (off_t) ((uin->uid - 1) * sizeof (record)), SEEK_SET);

  found = 0;
  if (read (fh, &record, sizeof (record)) > 0)
    {
      if (strcmp (cookie.id, record.userid) == 0)
	{
	  if (junkboard (board) == 0)
	    record.numposts++;

	  (void) lseek (fh, (off_t) (-1 * sizeof (record)), SEEK_CUR);
	  (void) write (fh, &record, sizeof (record));
	  found = 1;
	}
    }
  lockfile (fh, 0);
  (void) close (fh);
  if (found == 0)
    {
      printf ("Location: http://%s%s/bbslogin?status=3\n\n", BBSHOST, BBSCGI);
      exit (1);
    }



  now = time (NULL);
  sprintf (filename, "M.%ld.A", now);
  ptr = strrchr (filename, 'A');
  while (1)
    {
      sprintf (article, "%s/boards/%s/%s", BBSHOME, board, filename);
      fh = open (article, O_CREAT | O_EXCL | O_WRONLY, 0644);
      if (fh != -1)
	break;
      if (*ptr < 'z')
	(*ptr)++;
      else
	ptr++, *ptr = 'a', ptr[1] = '\0';
    }

  color = (record.numlogins % 7) + 31;
  sprintf (buf1,
	   "發信人: %s (%s), 信區: %s\n標  題: %s\n發信站: %s (%.24s) , %s\n\n",
	   noname == 1 ? "Anonymous" : record.userid,
	   noname == 1 ? "我是匿名天使" : record.username,
	   brdhdr.filename, title, BBSNAME, ctime (&now),
	   exchange == 1 ? "轉信" : "站內信件");
  (void) write (fh, buf1, strlen (buf1));
  buf2 = autocr (buf);
  (void) write (fh, buf2, strlen (buf2));
  free (buf2);
  (void) write (fh, "\n\n--\n", 5);

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
	    (void) write (fh, tmpsig[i - 1], strlen (tmpsig[i - 1]));
	}
    }

  if (HIDE_IP == 1)
    sprintf (buf1, "\x1b[1;%2dm※ 來源:．%s %s. [FROM: WWW] \x1b[m\n",
	     color, BBSNAME, BBSHOST);
  else
    sprintf (buf1,
	     "\x1b[1;%2dm※ 來源:．%sWWW %s. [FROM: %.20s] \x1b[m\n",
	     color, BBSNAME, BBSHOST,
	     noname == 1 ? "匿名天使的家" : getenv ("REMOTE_ADDR"));
  (void) write (fh, buf1, strlen (buf1));
  (void) close (fh);


  bzero ((void *) &header, sizeof (header));
  strcpy (header.filename, filename);
  if (exchange == 0)
    {
      header.filename[STRLEN - 1] = 'L';
      header.filename[STRLEN - 2] = 'L';
    }
  else
    {
      header.filename[STRLEN - 1] = 'S';
      header.filename[STRLEN - 2] = 'S';
    }
  strncpy (header.owner, noname == 1 ? "Anonymous" : record.userid, IDLEN);
  strncpy (header.title, title, STRLEN);
  lockfile (fileno (fidx), 1);
  (void) fseek (fidx, 0, SEEK_END);
  (void) fwrite (&header, sizeof (header), 1, fidx);
  lockfile (fileno (fidx), 0);
  (void) fclose (fidx);

  if (exchange == 1)
    {
      sprintf (filename, "%s/innd/out.bntp", BBSHOME);
      if ((fp = fopen (filename, "a")) != NULL)
	{
	  fprintf (fp, "%s\t%s\t%s\t%s\t%s\n", board, header.filename,
		   header.owner, record.username, header.title);
	  (void) fclose (fp);
	}
    }

  if (junkboard (board) == 0 && !brdhdr.level)
    {
      strcpy (postlog.author, record.userid);
      strcpy (postlog.board, board);
      ptr = header.title;
      if (strncmp (ptr, "Re: ", 4) == 0)
	ptr += 4;
      strncpy (postlog.title, ptr, 65);
      postlog.date = time (0);
      postlog.number = 1;
      sprintf (filename, "%s/.post", BBSHOME);
      if ((fh = open (filename, O_WRONLY | O_CREAT | O_APPEND)) > -1)
	{
	  (void) write (fh, &postlog, sizeof (postlog));
	  (void) close (fh);
	}
    }

  printf ("Location: http://%s%s/bbsdoc?board=%s%s\n\n", BBSHOST, BBSCGI,
	  board, cookie_string);

  cgi_quit ();
  return (0);
}
