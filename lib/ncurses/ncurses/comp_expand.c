/****************************************************************************
 * Copyright (c) 1998 Free Software Foundation, Inc.                        *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *  Author: Thomas E. Dickey <dickey@clark.net> 1998                        *
 ****************************************************************************/

#include <curses.priv.h>

#include <ctype.h>
#include <tic.h>

MODULE_ID("$Id: comp_expand.c 153052 2007-11-02 21:10:56Z coreos $")

static int trailing_spaces(const char *src)
{
	while (*src == ' ')
		src++;
	return *src == 0;
}

/* this deals with differences over whether 0x7f and 0x80..0x9f are controls */
#define CHAR_OF(s) (*(unsigned const char *)(s))
#define REALCTL(s) (CHAR_OF(s) < 127 && iscntrl(CHAR_OF(s)))
#define REALPRINT(s) (CHAR_OF(s) < 127 && isprint(CHAR_OF(s)))

char *_nc_tic_expand(const char *srcp, bool tic_format)
{
static char *	buffer;
static size_t	length;

int		bufp;
const char	*ptr, *str = VALID_STRING(srcp) ? srcp : "";
bool		islong = (strlen(str) > 3);
size_t		need = (2 + strlen(str)) * 4;
int		ch;

	if (buffer == 0) {
		buffer = malloc(length = need);
	} else if (need > length) {
		buffer = realloc(buffer, length = need);
	}

    	bufp = 0;
    	ptr = str;
    	while ((ch = (*str & 0xff)) != 0) {
		if (ch == '%' && REALPRINT(str+1)) {
	    		buffer[bufp++] = *str++;
	    		buffer[bufp++] = *str;
		}
		else if (ch == 128) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = '0';
		}
		else if (ch == '\033') {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'E';
		}
		else if (ch == '\\' && tic_format && (str == srcp || str[-1] != '^')) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = '\\';
		}
		else if (ch == ' ' && tic_format && (str == srcp || trailing_spaces(str))) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 's';
		}
		else if ((ch == ',' || ch == ':' || ch == '^') && tic_format) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = ch;
		}
		else if (REALPRINT(str) && (ch != ',' && ch != ':' && !(ch == '!' && !tic_format) && ch != '^'))
		    	buffer[bufp++] = ch;
#if 0		/* FIXME: this would be more readable (in fact the whole 'islong' logic should be removed) */
		else if (ch == '\b') {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'b';
		}
		else if (ch == '\f') {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'f';
		}
		else if (ch == '\t' && islong) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 't';
		}
#endif
		else if (ch == '\r' && (islong || (strlen(srcp) > 2 && str[1] == '\0'))) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'r';
		}
		else if (ch == '\n' && islong) {
	    		buffer[bufp++] = '\\';
	    		buffer[bufp++] = 'n';
		}
#define UnCtl(c) ((c) + '@')
		else if (REALCTL(str) && ch != '\\' && (!islong || isdigit(str[1])))
		{
			(void) sprintf(&buffer[bufp], "^%c", UnCtl(ch));
			bufp += 2;
		}
		else
		{
			(void) sprintf(&buffer[bufp], "\\%03o", ch);
			bufp += 4;
		}

		str++;
    	}

    	buffer[bufp] = '\0';
    	return(buffer);
}