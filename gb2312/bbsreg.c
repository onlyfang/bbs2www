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
$Id: bbsreg.c,v 1.12 2001/02/23 21:29:00 Only Exp $
*/

#include "bbs2www.h"

static char *userid, *passwd, *repass,
  *nickname, *realname, *career, *address,
  *email, *phone, *year, *month, *day;
static int gender;

static void
print_table (char *information)
{

  printf ("<form method=post action=\"%s/bbsreg\">\n", BBSCGI);
  printf ("<table class=title width=90%%><tr>");
  printf ("<th class=title align=center>%s -- ���û�ע��</th>\n", BBSNAME);
  printf ("</table>\n");
  printf ("<hr>\n");

  printf ("<table class=post>\n");
  printf ("<tr><td class=post align=center colspan=3>");
  if (information)
    printf ("%s\n", information);
  else
    printf ("���ʵ��д�����ע�����뵥.����������ٵ����Ͻ���������.\n");

  printf ("<tr><td class=post align=right>���������:\n");
  printf ("<td class=post colspan=2 align=left>");
  printf ("<input type=text name=userid size=%d", IDLEN);
  printf (" maxlength=%d value=\"%s\">\n", IDLEN, userid);

  printf ("<tr><td class=post align=right>���趨��������:\n");
  printf ("<td class=post colspan=2 align=left>");
  printf ("<input type=password name=passwd size=14 maxlength=%d", PASSLEN);
  printf (" value=\"%s\">\n", passwd);

  printf ("<tr><td class=post align=right>��������һ����������:\n");
  printf ("<td class=post colspan=2 align=left>");
  printf ("<input type=password name=repass size=14 maxlength=%d", PASSLEN);
  printf (" value=\"%s\">\n", repass);

  printf ("<tr><td class=post align=right>�����������ǳ�:\n");
  printf ("<td class=post colspan=2 align=left>");
  printf ("<input type=text name=nickname size=20 maxlength=%d", NAMELEN);
  printf (" value=\"%s\"> (����,С����)\n", nickname);

  printf ("<tr><td class=post align=right>������������ʵ����:\n");
  printf ("<td class=post colspan=2 align=left>");
  printf ("<input type=text name=realname size=20 maxlength=%d", NAMELEN);
  printf (" value=\"%s\"> (վ����������ܵ�!)\n", realname);

  printf ("<tr><td class=post align=right>���������ķ���λ:\n");
  printf ("<td class=post colspan=2 align=left>");
  printf ("<input type=text name=career size=50 maxlength=%d", STRLEN - 10);
  printf (" value=\"%s\">\n", career);

  printf ("<tr><td class=post align=right>����������ͨѶ��ַ:\n");
  printf ("<td class=post colspan=2 align=left>");
  printf ("<input type=text name=address size=50 maxlength=%d", STRLEN - 10);
  printf (" value=\"%s\">\n", address);

  printf ("<tr><td class=post align=right>�������������:\n");
  printf ("<td class=post colspan=2 align=left>");
  printf ("<input type=text name=email size=%d", NAMELEN);
  printf (" maxlength=%d value=\"%s\">\n", NAMELEN, email);

  printf ("<tr><td class=post align=right>����������绰:\n");
  printf ("<td class=post colspan=2 align=left>");
  printf ("<input type=text name=phone size=%d", NAMELEN);
  printf (" maxlength=%d value=\"%s\"> (����������ʱ��)\n", STRLEN, phone);

  printf ("<tr><td class=post align=right>��������������:\n");
  printf ("<td class=post align=left>");
  printf ("<input type=text name=year size=4 maxlength=4");
  printf (" value=\"%s\">��", year);
  printf ("<input type=text name=month size=2 maxlength=2");
  printf (" value=\"%s\">��", month);
  printf ("<input type=text name=day size=2 maxlength=2");
  printf (" value=\"%s\">��", day);

  printf ("<td class=post align=left>�����Ա���: ");
  printf ("<input type=radio name=gender value=0");
  if (gender == 0)
    printf (" checked");
  printf (">�� <input type=radio name=gender value=1");
  if (gender == 1)
    printf (" checked");
  printf (">Ů");

  printf ("</table><hr>\n");

  printf ("<input type=submit value=\"ע��\">");
  printf (" <input type=reset value=\"���\">\n");
  printf ("</form>\n");
  printf ("</center></body>\n</html>\n");

  cgi_quit ();
  exit (0);
}

int
main ()
{
  FILE *fp;
  char filename[STRLEN];
  int i, fh, total, birthyear, birthmonth, birthday;
  struct userec newuser;
  struct stat st;
  struct tm *tmnow;
  time_t now;


  cgi_init ();
  cgi_html_head ();
  printf ("<title>%s�û�ע��</title>\n", BBSID);
  printf ("<body><center>\n");

  userid = cgi_get ("userid");
  passwd = cgi_get ("passwd");
  repass = cgi_get ("repass");
  nickname = cgi_get ("nickname");
  realname = cgi_get ("realname");
  career = cgi_get ("career");
  address = cgi_get ("address");
  email = cgi_get ("email");
  phone = cgi_get ("phone");
  year = cgi_get ("year");
  month = cgi_get ("month");
  day = cgi_get ("day");
  gender = atoi (cgi_get ("gender"));

  if (cgi_num_entries == 0)
    print_table (NULL);

  if (userid[0] == '\0')
    print_table ("���������!");
  for (i = 0; userid[i] != '\0'; i++)
    if (!isalpha (userid[i]) && userid[i] != '_')
      print_table ("���ű���ȫΪӢ����ĸ!");
  if (strlen (userid) < 2)
    print_table ("����������������Ӣ����ĸ!\n");
  if (passwd[0] == '\0')
    print_table ("����������!");
  if (strlen (passwd) < 4 || strcmp (passwd, userid) == 0)
    print_table ("����̫�̻���ʹ���ߴ�����ͬ, ����������!");
  if (repass[0] == '\0')
    print_table ("��������������!");
  if (strcmp (passwd, repass) != 0)
    print_table ("������������벻һ��!");
  if (nickname[0] == '\0' || strlen (nickname) < 2)
    print_table ("�������ǳ�!");
  if (realname[0] == '\0' || strlen (realname) < 4)
    print_table ("��������ʵ����!");
  if (career[0] == '\0' || strlen (career) < 6)
    print_table ("�����뵥λ����!");
  if (address[0] == '\0' || strlen (address) < 6)
    print_table ("������ͨѶ��ַ!");

  if (email[0] == '\0')
    print_table ("�������������!");
  if (strchr (email, '@') == NULL || strchr (email, '.') == NULL)
    print_table ("���������ʽΪ: user@your.domain.name\n");

  if (phone[0] == '\0' || strlen (phone) < 4)
    print_table ("��������ϵ�绰!");
  if (year[0] == '\0' || month[0] == '\0' || day[0] == '\0')
    print_table ("��������������!");
  now = time (NULL);
  tmnow = localtime (&now);
  birthyear = atoi (year);
  birthmonth = atoi (month);
  birthday = atoi (day);
  if (birthyear < 100)
    birthyear += 1900;
  if (birthyear - 1900 < tmnow->tm_year - 98	/* Too old to BBS */
      || birthyear - 1900 > tmnow->tm_year - 3	/* Too young :-) */
      || birthmonth < 1 || birthmonth > 12 || birthday < 1 || birthday > 31)
    print_table ("��������������!");

  if (seek_in_file (".badname", userid)
      || seek_in_file ("etc/bad_id", userid))
    print_table ("ϵͳ���ֻ��ǲ��ŵĴ��š�");

  sprintf (filename, "%s/.PASSWDS", BBSHOME);
  if ((fh = open (filename, O_RDWR)) == -1)
    show_error ("Can not open .PASSWDS file!");

  (void) fstat (fh, &st);
  total = st.st_size / sizeof (newuser);

  for (i = 0; i < total; i++)
    {
      (void) read (fh, &newuser, sizeof (newuser));
      if (strcasecmp (userid, newuser.userid) == 0)
	{
	  (void) close (fh);
	  print_table ("���ʺ��Ѿ�����ʹ��,������ѡ��");
	}
    }

  memset (&newuser, 0, sizeof (newuser));

  strncpy (newuser.userid, userid, IDLEN);
  strncpy (newuser.username, nickname, NAMELEN);
  strncpy (newuser.realname, realname, NAMELEN);
  strncpy (newuser.email, email, STRLEN);
  strncpy (newuser.address, address, STRLEN);

  strcpy (newuser.passwd, genpasswd (passwd));
  strcpy (newuser.termtype, "vt100");
  strncpy (newuser.lasthost, getenv ("REMOTE_ADDR"), 15);
  newuser.lasthost[15] = '\0';
  newuser.numlogins = 1;
  newuser.numposts = 0;
  if (strcmp (newuser.userid, "guest") == 0)
    newuser.userlevel = 0;
  else if (strcmp (newuser.userid, "SYSOP") == 0)
    newuser.userlevel = ~0;
  else
    {
      if (CHECK_PERM)
	newuser.userlevel = PERM_BASIC;
      else
	newuser.userlevel = PERM_BASIC | PERM_DEFAULT;
    }
  newuser.firstlogin = newuser.lastlogin = time (NULL);
  newuser.userdefine = -1;
  newuser.flags[0] = CURSOR_FLAG | PAGER_FLAG;
  newuser.flags[1] = 0;

#ifdef FB30
  newuser.birthyear = atoi (year) - 1900;
  newuser.birthmonth = atoi (month);
  newuser.birthday = atoi (day);
  if (gender == 0)
    newuser.gender = 'M';
  else
    newuser.gender = 'F';
#endif

  lockfile (fh, 1);
  (void) lseek (fh, 0, SEEK_END);
  (void) write (fh, &newuser, sizeof (newuser));
  lockfile (fh, 0);
  (void) close (fh);

  if (CHECK_PERM)
    {
      sprintf (filename, "%s/new_register", BBSHOME);
      if ((fp = fopen (filename, "a")) != NULL)
	{
	  fprintf (fp, "usernum: %d, %s", total + 1, ctime (&now));
	  fprintf (fp, "userid: %s\n", userid);
	  fprintf (fp, "realname: %s\n", realname);
	  fprintf (fp, "career: %s\n", career);
	  fprintf (fp, "addr: %s\n", address);
	  fprintf (fp, "phone: %s\n", phone);
	  fprintf (fp, "birth: %s��%s��%s��\n", year, month, day);
	  fprintf (fp, "----\n");
	  (void) fclose (fp);
	}

      sprintf (filename, "%s/%s", BBSHOME, FLUSH);
      if ((fp = fopen (filename, "a")) != NULL)
	{
	  fprintf (fp, "touch by: %s", userid);
	  (void) fclose (fp);
	}
    }

  sprintf (filename, "%s/home/%c/%s", BBSHOME, toupper (userid[0]), userid);
  (void) mkdir (filename, 0755);
  sprintf (filename, "%s/mail/%c/%s", BBSHOME, toupper (userid[0]), userid);
  (void) mkdir (filename, 0755);

  printf ("<table cellspacing=0 class=title width=90%%><tr>");
  printf ("<th class=title align=center>%s -- ���û�ע��</th>\n", BBSNAME);
  printf ("</table>\n");
  printf ("<hr>\n");

  printf ("<table class=post>\n");
  printf ("<tr><td class=post colspan=2 align=left>");
  printf ("������! ����˳����ɱ�վ��ʹ����ע������,\n");
  printf ("վ�������������������ע������, �����ĵȺ�.\n");
  printf ("<tr><td class=post colspan=2 align=center>���Ļ�����������:\n");
  printf ("<tr><td class=post align=right>ʹ���ߴ��ţ�\n");
  printf ("<td class=post align=left>%s (%s)\n", userid, nickname);
  printf ("<tr><td class=post align=right>��  ����\n");
  printf ("<td class=post align=left>%s\n", realname);
  printf ("<tr><td class=post align=right>��վλ�ã�\n");
  printf ("<td class=post align=left>%s\n", newuser.lasthost);
  printf ("<tr><td class=post align=right>�����ʼ���\n");
  printf ("<td class=post align=left>%s\n", email);
  printf ("</table><hr>\n");

  printf ("<table class=foot>\n");
  print_go_loginout ();
  printf ("</table>\n");

  printf ("</center>\n");
  printf ("</body>\n");
  printf ("</html>");

  cgi_quit ();

  return (0);
}
