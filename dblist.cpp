/* dblist.cpp
vim: set ts=8 sw=8:

   Usage: dblist {-info} {-all} {-conv} {-escape} {-null} {-pg} {-raw} dbf_file ...
	-info	List dBase structures
	-all	All records, include deleted records
	-conv	Convert invalid BIG5 to '<?XXXX>'
	-escape	Escape(No) DBCS
	-ext=	Save as extension name to ???
	-first=	First record no.
	-last=	Last record no.
	-null	tNull if empty string when in PostgreSQL mode
	-pg	PostgreSQL COPY mode, always raw mode
	-raw	No heading & record number
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <fcntl.h>
#if defined(MSDOS) || defined(__MSDOS__)
    #include <io.h>
#endif
#include "dbase.h"

#if defined(__GNUC__)
#define	CompareTextLen	strncasecmp
#else
#define	CompareTextLen	strnicmp
#endif

static	int	listSchema	= 0;
static	int	allData		= 0;
static	int	checkBig5	= 0;
static	int	rawMode		= 0;
static	int	pgMode		= 0;
static	int	nullMode	= 0;
static	int	escapeMode	= 0;
static	char	delimiter	=',';
static	char	*saveExt	= NULL;

static long get_number ( const char *s )
{
	while ( *s && !isdigit(*s) ) s++;
	if ( isdigit(*s) ) return atol(s);
	return 0;
}


static int strLength ( const char *s, int len )
{
	while ( len > 0 && (unsigned char)s[len-1] <= ' ' ) len--;
	return len;
}


static const char * pgStr ( const char *s, int len )
{
static	char	*buf = NULL;
static	int	sizeBuf = 0;
	int	i;

	// not include tail spaces
	len = strLength(s,len);

	int	n = len+1;
	for ( i = 0 ; i < len ; i++ ) {
		unsigned char c = (unsigned char)s[i];
		if ( c < ' ' ) {
			n += 3;		/* Octal: \ooo */
		} else if ( !escapeMode && c >= 128 ) {
			n += 5;		/* worst case: "CC" -> "<?XXXX>" */
			i++;		/* double-character */
		} else if ( strchr("\'\"\\",c) != NULL ) {
			n++;		/* --> '"\ */
		}
	}

	if ( n > sizeBuf ) {
		if ( buf != NULL ) delete[] buf;
		sizeBuf = (n+0x3ff) & ~0x3ff;
		buf = new char[sizeBuf];
		if ( buf == NULL ) {
			fprintf(stderr,"Out of memory!\n");
			exit(1);
		}
	}

	n = 0;
	for ( i = 0 ; i < len ; i++ ) {
		unsigned char c = (unsigned char)s[i];
		if ( c < ' ' ) {	/* Octal: \ooo */
			buf[n++] = '\\';
			buf[n++] = '0' + ((c>>6) & 0x07);
			buf[n++] = '0' + ((c>>3) & 0x07);
			buf[n++] = '0' + (c&0x07);
		} else {
			if ( !escapeMode && c >= 128 ) {
				if ( i+1 < len ) {
					unsigned char c1;
					unsigned int cc;
					int	isStdBig5;
					c1 = c;
					c = (unsigned char)s[++i];
					if ( checkBig5 ) {
						cc = (c1 << 8) | c;
						isStdBig5 = (c1 >= 0xa1 && c1 <= 0xfe) &&
							((c >= 0x40 && c <= 0x7e) || (c >= 0xa1 && c <= 0xfe));
						if ( isStdBig5 ) {
							if ( (cc >= 0xc6a1 && cc <= 0xc8fe) ||	// 408 User1 Fonts
							     (cc >= 0xf9d6 && cc <= 0xf9fe) ||	//  41 ET Fonts
							     (cc >= 0xfa40 && cc <= 0xfefe) )	// 785 User2 Fonts
								isStdBig5 = 0;
						}
					} else {
						isStdBig5 = 1;
					}

					if ( isStdBig5 ) {
						buf[n++] = c1;
					} else {
						sprintf(&buf[n],"<?%04x",cc);
						n += 6;
						c = '>';
					}
				} else {
					c = '?';
				}
			} else if ( strchr("\'\"\\",c) != NULL ) {
				buf[n++] = '\\';
			}
			buf[n++] = c;
		}
	}

	if ( n == 0 && nullMode ) return "\\N";

	buf[n] = '\0';
	return buf;
}


static const char * trimall ( const char *s, int len, int pgMode )
{
static	char	buf[256];
	int	i = 0;

	while ( len > 0 && (unsigned char)s[len-1] <= ' ' ) len--;
	while ( i < len && (unsigned char)s[i] <= ' ' ) i++;
	len -= i;
	if ( len >= sizeof(buf) ) len = sizeof(buf) - 1;
	memcpy(buf,&s[i],len);
	buf[len] = '\0';
	return len == 0 && pgMode ? "\\N" : buf;
}


static int list_dbf ( const char *cFile, long nth, long num )
{
	FILE	*fpout;
	DBF	f;
	char	cDBF[256];

	memset(cDBF,0,sizeof(cDBF));
	strncpy(cDBF,cFile,sizeof(cDBF)-10);
	if ( f.open(cDBF) != 0 ) {
		strcat(cDBF,".dbf");
		f.open(cDBF);
	}
	if ( !f.isOpen() ) {
		fprintf(stderr,"\nCan't open dbase file: %s\n",cFile);
		return 1;
	}

	fpout = stdout;
	if (listSchema) {
		fprintf(fpout,">>> %s\n"
			      "| Last updated: %04d-%02d-%02d\n"
			      "|  Header size: %d\n"
			      "|  Record size: %d\n"
			      "| Record count: %ld\n",
			cDBF,
			f.updatedYear(),f.updatedMonth(),f.updatedDay(),
			f.headSize(),
			f.recSize(),
			f.recCount());
		fprintf(fpout,"No. Name       Type  Len  Dec\n"
			      "--- ---------- ---- ----- ---\n");
		for ( int i = 0 ; i < f.fieldCount() ; i++ ) {
			fprintf(fpout,"%2d. %-12s%c%7d%4d\n",
				i+1,
				f.fieldName(i),
				f.fieldType(i),
				f.fieldLength(i),
				f.fieldDecimal(i));
		}
		fprintf(fpout,"-----------------------------\n\n");

		return 0;
	}

	if ( saveExt && *saveExt != '\0' ) {
		fprintf(stderr,"Process %s ...\n",cDBF);

		char	*p = &cDBF[strlen(cDBF)];
		while ( --p >= &cDBF[0] && isalnum(*p) );
		if ( *p == '.' ) {
			strcpy(p+1,saveExt);
		} else {
			strcat(cDBF,".");
			strcat(cDBF,saveExt);
		}
		fpout = fopen(cDBF,rawMode?"wb":"w");
		if ( fpout == NULL ) {
			fprintf(stderr,"\nCan't create output file: %s\n",cDBF);
			fpout = stdout;
		}
	}

	if ( num == 0 )
		num = f.recCount();
	else
		num = num - nth + 1;

	if ( !rawMode ) {
		fprintf(fpout,"RecNo. ") ;
		for ( int i = 0 ; i < f.fieldCount() ; i++ ) {
			char	*p = f.fieldName(i) ;
			int	l = strlen(p) ;
			int	ll = f.fieldLength(i) ;
			if ( ll < l ) ll = l ;
			if ( f.fieldType(i) != 'N' )
				fprintf(fpout," %-*s",ll,p) ;
			else
				fprintf(fpout," %*s",ll,p) ;
		}
		fprintf(fpout,"\n") ;
	}

	if ( nth <= 0 ) nth = 0; else nth--;
	while ( num-- > 0 && ++nth <= f.recCount() ) {
		if ( f.read(nth) != DB_SUCCESS ) {
			fprintf(stderr,">>> #%ld *** reading error !\n",nth);
			break ;
		}

		if ( !allData && f.isDeleted() ) continue;

		if ( !rawMode ) {
			fprintf(fpout,"%-6ld%c",
				nth,f.isDeleted()?'*':' ') ;
		}
		for ( int i = 0 ; i < f.fieldCount() ; i++ ) {
			char	*p = f.fieldPtr(i) ;
			int	ll = f.fieldLength(i) ;
			int	isOverflowInt = 0;
			long	value;
			if ( f.fieldType(i) == 'N' && isalpha(*p) ) {
				isOverflowInt = 1;
				value = f.getLong(i);
			}
			if ( rawMode || pgMode ) {
				if ( i > 0 ) fprintf(fpout,"%c",delimiter);
				if ( f.fieldType(i) == 'C' ) {
					if ( pgMode )
						fprintf(fpout,"%s",pgStr(p,ll));
					else
						fprintf(fpout,"%.*s",strLength(p,ll),p);
				} else if ( isOverflowInt ) {
					fprintf(fpout,"%ld",value) ;
				} else {
					fprintf(fpout,"%s",trimall(p,ll,pgMode));
				}
			} else {
				int	l = strlen(f.fieldName(i)) ;
				if ( l < ll ) l = ll ;
				if ( f.fieldType(i) == 'C' ) {
					fprintf(fpout," %-*.*s",l,ll,p) ;
				} else if ( isOverflowInt ) {
					if ( l == ll ) l++;
					fprintf(fpout,"%*ld",l,value) ;
				} else {
					fprintf(fpout," %*.*s",l,ll,p) ;
				}
			}
		}
		fprintf(fpout,"\n") ;
	}

	if ( !rawMode ) {
		if ( nth <= f.recCount() )
			fprintf(fpout,"\nRecord Count = %ld\n",f.recCount()) ;
		else
			fprintf(fpout,"\n<* EOF *>\n") ;
	}

	if ( fpout != stdout ) fclose(fpout);

	return 0 ;
}


int main ( int argc, char *argv[] )
{
	int	i = 1;
	long	first=0;
	long	last=0;
	while ( i < argc && argv[i][0] == '-' ) {
		switch( tolower(argv[i][1]) ) {
		    case 'a':
			allData = 1;
			break;
		    case 'c':
			checkBig5 = 1;
			break;
		    case 'e':
			if ( CompareTextLen(&argv[i][1],"ext=",4) == 0 ) {
				saveExt = &argv[i][5];
				break;
			}
			escapeMode = 1;
			break;
		    case 'f':
			first = get_number(argv[i]);
			break;
		    case 'i':
			listSchema = 1;
			break;
		    case 'l':
			last = get_number(argv[i]);
			break;
		    case 'n':
			nullMode = 1;
			break;
		    case 'p':
			pgMode = 1;
			rawMode = 1;
			delimiter = '\t';
			break;
		    case 'r':
			rawMode = 1;
			break;
		    default:
			i = argc;
			break;
		}
		i++;
	}
	if ( argc <= i ) {
		printf("Usage: dblist {-info} {-all} {-conv} {-escape} {-null} {-pg} {-raw} dbf_file ...\n"
			"\t-info\tList dBase structures\n"
			"\t-all\tAll records, include deleted records\n"
			"\t-conv\tConvert invalid BIG5 to \'<?XXXX>\'\n"
			"\t-escape\tEscape(Not check) DBCS\n"
			"\t-ext=\tSave as extension name to ???\n"
			"\t-first=\tFirst record no.\n"
			"\t-last=\tLast record no.\n"
			"\t-null\tNull if empty string when in PostgreSQL mode\n"
			"\t-pg\tPostgreSQL COPY mode, always raw mode\n"
			"\t-raw\tNo heading & record number\n");
		return 1 ;
	}

#if defined(MSDOS) || defined(__MSDOS__)
	if ( rawMode ) setmode(fileno(stdout),O_BINARY);
#endif

	while ( i < argc ) {
		list_dbf(argv[i],first,last);
		i++;
	}
	return 0;
}
