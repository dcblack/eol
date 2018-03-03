/* **************************************************************

=pod

=head1 NAME

B<eol> - determines type of end of line characters in one or more files.

=head1 SYNOPSIS

B<eol> C<[-q][-c][-v][-w]> [-0] I<[FILELIST]>

B<eol> -V

=head1 DESCRIPTION

B<eol> echoes its results to STDOUT as a string of + separated fields
as follows:

=over

=item B<8BIT>

Indicates characters with ASCI values greater than 127 found.

=item B<CR>

Indicates ASCII carriage return (CR) character (0x13) found.

=item B<CRLF>

Indicates B<CR> immediately followed by LF found. Mutually exclusive from B<CR>
or B<LF> alone.

=item B<CTRL>

Indicates control characters other than LF,CR,TAB,NUL,^Z.

=item B<LF>

Indicates ASCII line feed (LF) character (0x10) found.

=item B<LONG>

Indicates at least one line greater than 132 chars.

=item B<NONE>

Indicates neither CRLF, CR nor LF found.

=item B<TAB>

Indicates one or more TAB characters found. In parentheses
is the number of tabs found, and the maximum number of tabs
in a single line.

=item B<DEL>

Indicates one or more bytes containing 127 (0x7F) found.

=item B<NULL>

Indicates one or more bytes containing zero (0x00) found.
In parentheses is the number of nulls found.

=item B<WIDE>

Indicates at least one line between 80 & 132 chars.

=back

=head1 OPTIONS

=over

=item -0

Report the locations of the first several NUL's found (single file only).

=item C<-c>

Display count statistics for various elements (e.g. # tabs, nulls, etc).

=item C<-h>

Basic usage.

=item C<-q>

Quick check based on the first 1024 characters.

=item C<-v>

Display verbosely. Includes the maximum line widths.

=back

=head1 EXAMPLES

% B<eol> C<-q> *txt b*
 dos.txt CRLF
 mac.txt CR
 unix.txt LF
 binary LONG+8BIT+CTRL+NULL+CR+LF
 binary2 LONG+8BIT+CTRL+NULL+CR+LF

=head1 AUTHOR

David C Black

=cut

 ************************************************************** */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static char const rcsid[] =
    "$Id: eol.c,v 1.0 2018/02/28 22:51:37 dcblack Exp $";
int verbose = 0;
int debug = 0;
int tokens = 0;
int counts = 0;
char* ignore;

int stat();

#define MAXNULLS  16
#define MAXTEST 1024
#define MAXLINE  132 /* maximum allowable line length */

long int incr( long int* value )
{
    long int result = ( *value ) + 1;

    if ( result > 0 ) {
        *value = result;    // don't allow overflow
    }

    return *value;
}

long long size = 0;
int prev_c;
long int line_len;
long int line_max;
long int token_len;
long int token_max;
long int ident_len;
long int ident_max;
long int nuls; /* Number of "\0"s   encountered */
long int tabs; /* Number of "\t"s   encountered */
long int tabc; /* Number of "\t"s   since last EOL */
long int tabm; /* Max number of "\t"s between EOL's */
long int dels; /* Number of "\377"s encountered */
long int bit8; /* Number of >127's  encountered */
long int ctls; /* Number of <32's   encountered */
long int cr;   /* Number of "\r"s   encountered */
long int lf;   /* Number of "\n"s   encountered */
long int cl;   /* Number of "\r\n"s encountered */
short high_bit;
short ctrl_char;

void reset_count( void )
{
    prev_c = -1;
    line_len = 0L;
    line_max = 0L;
    token_len = 0L;
    token_max = 0L;
    ident_len = 0L;
    ident_max = 0L;
    nuls = 0L; /* Number of "\0"s   encountered */
    tabs = 0L; /* Number of "\t"s   encountered */
    tabc = 0L;
    tabm = 0L;
    dels = 0L; /* Number of "\377"s encountered */
    bit8 = 0L;
    ctls = 0L;
    cr = 0L;   /* Number of "\r"s     encountered */
    lf = 0L;   /* Number of "\n"s     encountered */
    cl = 0L;   /* Number of "\r\n"s   encountered */
    high_bit = 0;
    ctrl_char = 0;
    counts = 0;
}//end reset_count()

void count_tabs( void )
{
    if ( tabm < tabc ) {
        tabm = tabc;
    }

    tabc = 0;
}//end count_tabls()

void findeol( int maxtest, int findnulls, FILE* fp )
{
    int c;
    int i = 0;

    while ( ( c = fgetc( fp ) ) != EOF && ( maxtest == 0 || ++i < maxtest ) ) {
        if ( line_len >  line_max ) {
            line_max =  line_len;
        }

        if ( token_len > token_max ) {
            token_max = token_len;
        }

        if ( ident_len > ident_max ) {
            ident_max = ident_len;
        }

        if ( c > 127 ) { /* Bit 8 is set */
            high_bit = 1;
            incr( &bit8 );
        }/*endif*/

        if ( c == '\377' ) { /* DEL character detected */
            incr( &dels );
        }

        if ( isspace( c ) && !isspace( prev_c ) ) { /* whitespace */
            token_len = 0L;
        }

        if ( !isalnum( c ) && isalnum( prev_c ) ) { /* whitespace */
            ident_len = 0L;
        }

        if ( isalnum( c ) ) { /* identifier or number */
            incr( &ident_len );
        }

        if ( c > 32 ) { /* Printable */
            incr( &line_len );
            incr( &token_len );
        }
        else {
            switch ( c ) {
                case '\r': /* <CARRIAGE-RETURN> */
                    incr( &cr );
                    count_tabs();
                    line_len = 0L;
                    break;

                case '\n': /* <LINE-FEED> */
                    if ( prev_c == '\r' ) {
                        incr( &cl );
                        /* assert cr > 0 */
                        cr--;
                    }
                    else {
                        incr( &lf );
                    }/*endif*/

                    count_tabs();
                    line_len = 0L;
                    break;

                case '\000': /* <NUL> */
                    incr( &nuls );

                    if ( findnulls-- > 0 ) {
                        int lineno = cr + lf + cl;
                        ( void ) printf( "Null in column %ld on line %d\n", line_len, lineno );
                    }/*endif*/

                    incr( &line_len );
                    break;

                case '\t': /* <TAB> */
                    incr( &tabs );
                    incr( &tabc );
                    incr( &line_len );
                    break;

                case ' ': /* <SPACE> */
                    incr( &line_len );
                    break;

                case '\v':   /* <VERTICAL-TAB> */
                case '\b':   /* <BACK-SPACE> */
                case '\f':   /* <FORM-FEED> */
                    incr( &line_len );
                    break;

                default:
                    incr( &ctls );
                    ctrl_char = 1;
                    break;
            }/*endswitch*/
        }/*endif*/

        prev_c = c;
    }//endwhile

}//end findeol()

void results( char msg[1024] )
{
    ( void ) strcpy( msg, "" );
    char tmp[20];

    if ( prev_c == '\r' ) {
        cr--;    /* in case we stopped on a CR-LF boundary */
    }

    if ( line_max > 132 ) {
        ( void ) strcat( msg, "LONG" );

        if ( verbose ) {
            ( void ) sprintf( tmp, "%ld", line_max );
            ( void ) strcat( msg, tmp );
        }/*endif*/

        ( void ) strcat( msg, "+" );
    }
    else if ( line_max >  80 ) {
        ( void ) strcat( msg, "WIDE" );

        if ( verbose ) {
            ( void ) sprintf( tmp, "%ld", line_max );
            ( void ) strcat( msg, tmp );
        }/*endif*/

        ( void ) strcat( msg, "+" );
    }/*endif*/

    if ( tokens != 0 ) {
        ( void ) sprintf( tmp, "WL(%ld)", token_max );
        ( void ) strcat( msg, tmp );
        ( void ) sprintf( tmp, "IL(%ld)", ident_max );
        ( void ) strcat( msg, tmp );
        ( void ) strcat( msg, "+" );
    }/*endif*/

    if ( high_bit ) {
        ( void ) strcat( msg, "8BIT" );

        if ( counts ) {
            ( void ) sprintf( tmp, "(%ld)", bit8 );
            ( void ) strcat( msg, tmp );
        }

        ( void ) strcat( msg, "+" );
    }

    if ( ctrl_char ) {
        ( void ) strcat( msg, "CTRL" );

        if ( counts ) {
            ( void ) sprintf( tmp, "(%ld)", ctls );
            ( void ) strcat( msg, tmp );
        }

        ( void ) strcat( msg, "+" );
    }

    if ( nuls > 0 ) {
        ( void ) strcat( msg, "NULL" );

        if ( counts ) {
            ( void ) sprintf( tmp, "(%ld)", nuls );
            ( void ) strcat( msg, tmp );
        }

        ( void ) strcat( msg, "+" );
    }

    if ( tabs > 0 ) {
        ( void ) strcat( msg, "TAB" );

        if ( counts ) {
            ( void ) strcat( msg, "(" );
            ( void ) sprintf( tmp, "%ld/%ld", tabs, tabm );
            ( void ) strcat( msg, tmp );
            ( void ) strcat( msg, ")" );
        }

        ( void ) strcat( msg, "+" );
    }

    if ( dels > 0 ) {
        ( void ) strcat( msg, "DEL+" );
    }

    if ( cr == 0 && lf == 0 && cl == 0 ) {
        ( void ) strcat( msg, "NONE" );
    }
    else if ( cr != 0 && lf == 0 && cl == 0 ) {
        ( void ) strcat( msg, "CR" );
    }
    else if ( cr == 0 && lf != 0 && cl == 0 ) {
        ( void ) strcat( msg, "LF" );
    }
    else if ( cr == 0 && lf == 0 && cl != 0 ) {
        ( void ) strcat( msg, "CRLF" );
    }
    else if ( cr != 0 && lf != 0 && cl == 0 ) {
        ( void ) strcat( msg, "CR+LF" );
    }
    else if ( cr != 0 && lf == 0 && cl != 0 ) {
        ( void ) strcat( msg, "CR+CRLF" );
    }
    else if ( cr == 0 && lf != 0 && cl != 0 ) {
        ( void ) strcat( msg, "LF+CRLF" );
    }
    else if ( cr != 0 && lf != 0 && cl != 0 ) {
        ( void ) strcat( msg, "CR+LF+CRLF" );
    }
}//end results

void testeol( char msg[1024] , int maxtest , int findnulls , FILE* fp )
{
    reset_count();

    if ( debug && maxtest ) {
        printf( "DEBUG: Sampling start\n" );
    }

    findeol( maxtest, findnulls, fp );

    if ( maxtest && ( size > 5 * maxtest ) ) {
        // test middle
        if ( debug ) {
            printf( "DEBUG: Sampling middle\n" );
        }

        fseek( fp, size / 2 - maxtest / 2, SEEK_SET );
        findeol( maxtest, findnulls, fp );

        // random seeks
        if ( size > 1000 * maxtest ) {
            int samples = 0;

            while ( samples < 10 ) {
                long int offset = rand() % size;

                if ( offset < 2 * maxtest || offset > size - 2 * maxtest ) {
                    continue;
                }

                samples++;

                if ( debug ) {
                    printf( "DEBUG: Sampling %ld\n", offset );
                }

                fseek( fp, offset, SEEK_SET );
            }//endwhile
        }//endif

        // test end
        if ( debug ) {
            printf( "DEBUG: Sampling end\n" );
        }

        fseek( fp, -maxtest, SEEK_END );
        findeol( maxtest, findnulls, fp );
    }//endif

    results( msg );
}//end testeol()

int main( int argc, char* argv[] )
{
    FILE* fp;
    short errors = 0;
    char msg[1024];
    int maxtest = 0;
    int findnulls = 0;
    struct stat fs;

    /* check options */
    while ( argc > 1 && ( char* ) index( argv[1], '-' ) == ( char* ) argv[1] ) {
        if ( strcmp( argv[1], "-q" ) == 0 ) {
            maxtest = MAXTEST; /* request quick test */
        }
        else if ( strcmp( argv[1], "-d" ) == 0 ) {
            debug = 1;
        }
        else if ( strcmp( argv[1], "-h" ) == 0 ) {
            ( void ) printf( "USAGE: eol [-q] [-0] [FILELIST]\n" );
            return ( 0 );
        }
        else if ( strcmp( argv[1], "-help" ) == 0 ) {
            return ( system( "man eol" ) );
        }
        else if ( strcmp( argv[1], "-0" ) == 0 ) {
            findnulls = MAXNULLS; /* request verbose null printout */
        }
        else if ( strcmp( argv[1], "-c" ) == 0 ) {
            counts = 1; /* request printing count statistics */
        }
        else if ( strcmp( argv[1], "-v" ) == 0 ) {
            verbose = 1; /* request verbose printout */
        }
        else if ( strcmp( argv[1], "-w" ) == 0 ) {
            tokens = 1; /* request token length statistics */
        }
        else if ( strcmp( argv[1], "-V" ) == 0 ) {
            return printf( "%s\n", rcsid ) == EOF;
        }
        else if ( strcmp( argv[1], "-a" ) == 0 ) {
            counts = 1; /* request printing count statistics */
            verbose = 1; /* request verbose printout */
        }
        else {
            ( void ) printf( "ERROR(eol): Unknown option %s ignored!\n", argv[1] );
        }/*endif*/

        argc--;            /* decrease arg count */
        argv++;            /* move to next arg   */
    }/*endwhile*/

    if ( argc == 1 ) { /* no args; copy standard input */
        testeol( msg, maxtest, findnulls, stdin );
        ( void ) printf( "%s\n", msg );
    }
    else {
        while ( --argc > 0 ) {
            if ( stat( *++argv, &fs ) < 0 ) {
                /* No such file! */
                ( void ) sprintf( msg, "eol %s", *argv );
                perror( msg );
                continue;
            }

            if ( ( fs.st_mode & S_IFMT ) == S_IFDIR ) {
                /* Directory */
                ( void ) printf( "%s DIRECTORY\n", *argv );
            }
            else if ( ( fs.st_mode & S_IFMT ) != S_IFREG ) {
                /* Not normal */
                ( void ) fprintf( stderr, "eol %s: not a regular file\n", *argv );
                errors++;
            }
            else if ( ( fp = fopen( *argv, "r" ) ) == NULL ) {
                /* Unreadable */
                ( void ) fprintf( stderr, "eol %s: can't open\n", *argv );
                errors++;
            }
            else {
                size = fs.st_size;
                testeol( msg, maxtest, findnulls, fp );
                ( void ) printf( "%s %s\n", *argv, msg );
                ( void ) fclose( fp );
            }
        }/*endwhile*/
    }/*endif*/

    return ( errors );
}
