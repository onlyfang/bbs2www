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
$Id: article.h,v 1.3 2001/02/23 21:28:59 Only Exp $
*/

#ifndef _ARTICLE_H_
#define _ARTICLE_H_

#define BRC_MAXSIZE	50000
#define BRC_MAXNUM	60
#define BRC_STRLEN	15
#define	BRC_ITEMSIZE	(BRC_STRLEN + 1 + BRC_MAXNUM * sizeof( int ))

extern char *brc_getrecord (char *ptr, char *name, int *pnum, int *list);
extern int brc_unread (char *filename, int brc_num, int *brc_list);

extern void delete_file (char *dirname, char *filename);
extern void display_article (FILE * fp, int allow_html);

#endif /* _ARTICLE_H_ */
