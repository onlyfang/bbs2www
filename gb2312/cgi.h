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
$Id: cgi.h,v 1.6 2001/02/23 21:29:00 Only Exp $
*/

#ifndef _CGI_H_
#define _CGI_H_

typedef struct entry_type
{
  char *name;
  char *value;
}
cgi_entry_type;

typedef struct cookie_type
{
  char id[20];
#ifdef ENCPASSLEN
  char code[ENCPASSLEN];
#else
  char code[PASSLEN];
#endif
  int level;
}
cgi_cookie_type;


extern cgi_entry_type *cgi_entries;
extern int cgi_num_entries;
extern int cgi_request_method;	/* 0 == GET; 1 == POST */
extern int cgi_text_html;
extern cgi_cookie_type cookie;
extern char cookie_string[STRLEN];

extern void show_error (char *reason);
extern void cgi_html_head (void);
extern void login_check ();
extern void cgi_init (void);
extern void cgi_quit (void);
extern char *cgi_get (const char *field_name);

#endif /* _CGI_H_ */
