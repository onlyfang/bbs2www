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
$Id: article.c,v 1.6 2001/02/23 21:28:59 Only Exp $
*/

#include "bbs2www.h"

void
delete_file (char *dirname, char *filename)
{
  int fd, total = 0, i;
  struct fileheader *buffer;
  struct stat st;

  if ((fd = open (dirname, O_RDWR)) == -1)
    return;

  lockfile (fd, 1);
  (void) fstat (fd, &st);
  total = st.st_size / sizeof (struct fileheader);
  buffer = (struct fileheader *) calloc (total, sizeof (struct fileheader));
  if (buffer == NULL)
    return;
  if (read (fd, buffer, (size_t) st.st_size) < (ssize_t) st.st_size)
    {
      free (buffer);
      return;
    }

  for (i = 0; i < total; i++)
    {
      if (strcmp (buffer[i].filename, filename) == 0)
	break;
    }

  while (i < total - 1)
    {
      memcpy ((void *) (buffer + i), (void *) (buffer + i + 1),
	      sizeof (struct fileheader));
      i++;
    }
  total--;

  (void) lseek (fd, 0, SEEK_SET);
  (void) write (fd, buffer, sizeof (struct fileheader) * total);

  free (buffer);
  lockfile (fd, 0);
  close (fd);
}

char *
brc_getrecord (char *ptr, char *name, int *pnum, int *list)
{
  int num;
  char *tmp;

  strncpy (name, ptr, BRC_STRLEN);
  ptr += BRC_STRLEN;
  num = (*ptr++) & 0xff;
  tmp = ptr + num * sizeof (int);
  if (num > BRC_MAXNUM)
    num = BRC_MAXNUM;
  *pnum = num;
  memcpy (list, ptr, num * sizeof (int));
  return tmp;
}

int
brc_unread (char *filename, int brc_num, int *brc_list)
{
  int ftime, n;

  ftime = atoi (&filename[2]);
  if ((filename[0] != 'M' && filename[0] != 'G') || filename[1] != '.')
    return 0;
  if (brc_num <= 0)
    return 1;
  for (n = 0; n < brc_num; n++)
    {
      if (ftime > brc_list[n])
	return 1;
      else if (ftime == brc_list[n])
	return 0;
    }

  return 0;
}

void
display_article (FILE * fp, int allow_html)
{
  char buf[512], tmp[80], *ptr;
  int index = 0, n, len, quote = 1, infont = 0, inblink = 0,
    inbold = 0, inunderline = 0, signature = 0;

  printf ("<pre>\n");

  while (fgets (buf, 512, fp) != NULL)
    {
      index = 0;
      if (quote == 1 && quote_string (buf) == 1)
	{
	  printf ("<font class=col33>%s</font><br>", buf);
	  quote = 0;
	}
      else
	{
	  if (strcmp (buf, "--\n") == 0)
	    signature = 1;
	  if (buf[0] == ':' || buf[0] == '>')
	    printf ("<font class=col36>");
	  while (buf[index] != '\0')
	    {
	      if (buf[index] != '\x1b' || buf[index + 1] != '[')
		{
		  if (allow_html == 0 && signature == 0 && buf[index] == '<')
		    printf ("&lt;");
		  else if (allow_html == 0 && signature == 0
			   && buf[index] == '>')
		    printf ("&gt;");
		  else
		    printf ("%c", buf[index]);
		  index++;
		}
	      else
		{
		  index += 2;
		  n = 0;
		  while (buf[index + n] != 'm' && buf[index + n] != '\0')
		    n++;
		  if (buf[index + n] == 'm')
		    {
		      len = (n > 79) ? 79 : (n + 1);
		      strncpy (tmp, buf + index, (size_t) len);
		      tmp[len] = '\0';
		      index += n + 1;
		      if (tmp[0] == 'm')
			strcpy (tmp, "0;");
		      else
			tmp[len - 1] = ';';
		      ptr = strtok (tmp, ";");
		      while (ptr)
			{
			  n = atoi (ptr);
			  switch (n)
			    {
			    case 0:
			      if (infont == 1)	/* 狀態還原 */
				printf ("</font>");
			      if (inblink == 1)
				printf ("</blink>");
			      if (inbold == 1)
				printf ("</b>");
			      if (inunderline == 1)
				printf ("</u>");
			      infont = inblink = inbold = inunderline = 0;
			      break;
			    case 1:
			      if (inbold == 0)	/* 高亮度 */
				printf ("<b>");
			      inbold = 1;
			      break;
			    case 4:
			      if (inunderline == 0)	/* 下划線 */
				printf ("<u>");
			      inunderline = 1;
			      break;
			    case 5:
			      if (inblink == 0)	/* 閃爍 */
				printf ("<blink>");
			      inblink = 1;
			      break;
			    case 30:
			      if (infont == 1)	/* 黑色 */
				printf ("</font>");
			      printf ("<font class=col30>");
			      infont = 1;
			      break;
			    case 31:
			      if (infont == 1)	/* 紅色 */
				printf ("</font>");
			      printf ("<font class=col31>");
			      infont = 1;
			      break;
			    case 32:
			      if (infont == 1)	/* 綠色 */
				printf ("</font>");
			      printf ("<font class=col32>");
			      infont = 1;
			      break;
			    case 33:
			      if (infont == 1)	/* 黃色 */
				printf ("</font>");
			      printf ("<font class=col33>");
			      infont = 1;
			      break;
			    case 34:
			      if (infont == 1)	/* 藍色 */
				printf ("</font>");
			      printf ("<font class=col34>");
			      infont = 1;
			      break;
			    case 35:
			      if (infont == 1)	/* 粉紅色 */
				printf ("</font>");
			      printf ("<font class=col35>");
			      infont = 1;
			      break;
			    case 36:
			      if (infont == 1)	/* 淺藍色 */
				printf ("</font>");
			      printf ("<font class=col36>");
			      infont = 1;
			      break;
			    case 37:
			      if (infont == 1)	/* 白色 */
				printf ("</font>");
			      printf ("<font class=col37>");
			      infont = 1;
			      break;
			    }
			  ptr = strtok (NULL, ";");
			}

		    }
		  else
		    {
		      index += n;
		    }

		}
	    }
	  if (buf[0] == ':' || buf[0] == '>')
	    printf ("</font>");

	}
    }
  if (infont == 1)
    printf ("</font>");
  if (inblink == 1)
    printf ("</blink>");
  if (inbold == 1)
    printf ("</b>");
  if (inunderline == 1)
    printf ("</u>");

  printf ("</pre>\n");
}
