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
$Id: cgi.c,v 1.8 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

cgi_entry_type *cgi_entries = NULL;
int cgi_num_entries = 0;
int cgi_request_method = 0;	/* 0 == GET; 1 == POST */
char *query = NULL;
int cgi_text_html = 0;
cgi_cookie_type cookie;
char cookie_string[STRLEN];

static char
x2c (char *what)
{
  register char digit;

  digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A') + 10 : (what[0] - '0'));
  digit *= 16;
  digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A') + 10 : (what[1] - '0'));
  return (digit);
}


static void
unescape_url (char *url)
{
  register int x, y, len;

  len = strlen (url);

  for (x = 0, y = 0; url[y]; ++x, ++y)
    {
      if ((url[x] = url[y]) == '%' && y < len - 2)
	{
	  url[x] = x2c (&url[y + 1]);
	  y += 2;
	}
    }
  url[x] = '\0';
}


static void
plustospace (char *str)
{
  register int x;

  for (x = 0; str[x]; x++)
    if (str[x] == '+')
      str[x] = ' ';
}

static void
cgi_free ()
{
  if (cgi_request_method == 1 && query != NULL)
    free (query);

  if (cgi_num_entries > 0 && cgi_entries != NULL)
    free (cgi_entries);
}

void
cgi_html_head ()
{
  printf ("Content-type: text/html; charset=%s\n\n", CHARSET);
  printf ("<html>\n");

#ifdef STYLESHEET
  printf ("<link rel=stylesheet type=text/css href=\"%s\">\n", STYLESHEET);
#endif

  cgi_text_html = 1;

}

void
show_error (char *reason)
{
  if (reason != NULL)
    {
      if (cgi_text_html == 0)
	cgi_html_head ();
      printf ("<h1>Error</h1>\n");
      printf ("%s\n", reason);
      printf ("</html>");
    }

  cgi_free ();
  exit (0);
}

void
login_check ()
{
  char *level, buf[256], filename[256];
  int i, fh, found, status = 0;
  struct user_info *uin;
  struct stat st;
  time_t now;

  strcpy (cookie.id, cgi_get ("id"));
  strcpy (cookie.code, cgi_get ("code"));
  level = cgi_get ("level");

  status = 0;

  while (1)
    {

      if (cookie.id[0] == '\0' || strcasecmp (cookie.id, "guest") == 0)
	break;

      status = 1;

      if (cookie.code[0] == '\0')
	break;

      resolve_utmp ();
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

      sprintf (filename, "%s/home/%c/%s/.wwwlogin", BBSHOME,
	       toupper (cookie.id[0]), cookie.id);
      if (!found)
	{
	  for (i = 0; i < USHM_SIZE; i++)
	    {
	      uin = &(utmpshm->uinfo[i]);
	      if (!uin->active || !uin->pid)
		{
		  found = 1;
		  break;
		}
	    }
	  if (!found)
	    show_error ("在线用户太多,超出了系统负荷! 请稍侯再试");
	  if ((fh = open (filename, O_RDONLY)) == -1)
	    break;


	  if (read (fh, uin, sizeof (struct user_info)) !=
	      sizeof (struct user_info))
	    show_error
	      ("Internel error. Can not read login record. Report to SYSOP");
	  (void) close (fh);
	}

      if (strncmp (uin->from, getenv ("REMOTE_ADDR"), 59) != 0)
	break;
      cookie.level = uin->pid ^ atoi (level);
      if (cookie.level <= 0)
	break;

      sprintf (buf, "%d%s", atoi (level) | uin->uid, uin->realname);

      buf[PASSLEN - 1] = '\0';
      if (!checkpasswd (cookie.code, buf))
	break;

      (void) stat (filename, &st);
      now = time (NULL);
      if ((now - st.st_atime > 1800 || now - st.st_mtime > 1800)
	  && (now - uin->idle_time > 1800))
	{
	  (void) remove (filename);
	  status = 4;
	  break;
	}
      else
	{
	  uin->idle_time = now;
	}

      status = 2;
      break;
    }

  if (status == 0)
    {
      strcpy (cookie.id, "guest");
      strcpy (cookie.code, "guest");
      cookie.level = 0;
      cookie_string[0] = '\0';
    }
  else if (status == 2)
    sprintf (cookie_string, "&id=%s&code=%s&level=%s", cookie.id, cookie.code,
	     level);
  else
    {
      printf ("Location: http://%s%s/bbslog?userid=%s&status=%d\n\n",
	      BBSHOST, BBSCGI, cookie.id, status);
      exit (0);
    }
}

void
cgi_init (void)
{
  int cl = 0, i;
  char *env;

  cgi_num_entries = 0;


  env = getenv ("REQUEST_METHOD");
  if (env == NULL)
    show_error ("No REQUEST_METHOD found!");

  if (strcmp (env, "POST") == 0)
    {
      cgi_request_method = 1;
      env = getenv ("CONTENT_TYPE");

      if (env == NULL
	  || strcmp (env, "application/x-www-form-urlencoded") != 0)
	show_error ("Incorrect CONTENT_TYPE!");
      else
	{
	  env = getenv ("CONTENT_LENGTH");
	  if (env == NULL || (cl = atoi (env)) <= 0)
	    show_error ("Bad CONTENT_LENGTH!");

	  query = malloc (cl + 1);

	  if (query == NULL)
	    show_error ("Out of memory!");

	  fgets (query, cl + 1, stdin);

	  if (strlen (query) != cl)
	    show_error ("CONTENT_LENGTH discrepancy!");
	}
    }
  else
    {
      cgi_request_method = 0;

      query = getenv ("QUERY_STRING");

      if (query == NULL)
	show_error ("No QUERY_STRING found!");
      else
	cl = strlen (query);

    }

  cgi_num_entries = 0;


  if (query != NULL)
    for (i = 0; i <= cl; i++)
      if (query[i] == '&' || query[i] == '\0')
	cgi_num_entries++;

  if (cgi_num_entries > 0)
    {
      cgi_entries = malloc (sizeof (cgi_entry_type) * cgi_num_entries);

      if (cgi_entries == NULL)
	show_error ("Out of memory!");

      cgi_num_entries = 0;


      if (query[0] != '\0' && query[0] != '&')
	cgi_entries[0].name = query;

      for (i = 0; i <= cl; i++)
	{
	  if (query[i] == '&')
	    {
	      cgi_entries[cgi_num_entries].name = query + i + 1;
	      query[i] = '\0';
	    }
	  else if (query[i] == '=')
	    {
	      cgi_entries[cgi_num_entries].value = query + i + 1;

	      cgi_num_entries++;

	      query[i] = '\0';
	    }
	}

      for (i = 0; i < cgi_num_entries; i++)
	{
	  plustospace (cgi_entries[i].value);
	  unescape_url (cgi_entries[i].value);
	  if (cgi_entries[i].name == NULL)
	    cgi_entries[i].name = "";
	  if (cgi_entries[i].value == NULL)
	    cgi_entries[i].value = "";
	  if (strstr (cgi_entries[i].value, "..") != NULL
	      && strstr (cgi_entries[i].name, "code") == NULL
	      && strstr (cgi_entries[i].name, "passwd") == NULL
	      && strstr (cgi_entries[i].name, "title") == NULL
	      && strstr (cgi_entries[i].name, "text") == NULL)
	    show_error ("Error input!");
	}
    }

}


void
cgi_quit (void)
{
  cgi_free ();
  cgi_entries = NULL;
  cgi_num_entries = 0;
  cgi_request_method = 0;
  query = NULL;
}

char *
cgi_get (const char *field_name)
{
  int x;

  for (x = 0; x < cgi_num_entries; x++)
    if (strcmp (cgi_entries[x].name, field_name) == 0)
      return (cgi_entries[x].value);

  return ("");
}
