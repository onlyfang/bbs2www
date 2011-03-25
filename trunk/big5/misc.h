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
$Id: misc.h,v 1.10 2001/02/23 21:29:01 Only Exp $
*/

#ifndef _MISC_H_
#define _MISC_H_

#define PERM(x)         ((x)?record.userlevel&(x):1)

extern void lockfile (int fd, int lock);
extern int dashf (char *fname);
extern int dashd (char *fname);
extern void uppercase (char *word);
extern char last_char (char *str);
extern void print_title (char *title);
extern void print_user (char *user);
extern int quote_string (char *buf);
extern int countexp (struct userec *udata);
extern int countperf (struct userec *udata);
extern int compute_user_value (struct userec *urec);
extern char *cexp (int exp);
extern char *cperf (int perf);
extern char *ModeType (int mode);
extern char *genpasswd (char *passwd);
extern int *checkpasswd (char *passwd, char *test);
extern int seek_in_file (char *filename, char *seek_str);

extern void resolve_utmp ();

extern void print_go_bbsman ();
extern void print_go_bbssec ();
extern void print_go_bbsall ();
extern void print_go_bbs0an ();
extern void print_go_bbsmil ();
extern void print_go_loginout ();

extern struct UTMPFILE *utmpshm;
#endif /* _MISC_H_ */
