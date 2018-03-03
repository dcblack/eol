/*************************************************************
 *	cat2 - concatentate files to standard error
 *
 *************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

int stat();

void cp2stderr( fp )
FILE* fp;
{
    int c;

    while ( ( c = fgetc( fp ) ) != EOF ) {
        fputc( c, stderr );
    }

}

int main( int argc, char* argv[] )
{
    FILE* fp;
    int errors = 0;
    struct stat fs;
    char msg[1024];

    if ( argc == 1 ) { /* no args; copy standard input */
        cp2stderr( stdin );
    }
    else {
        while ( --argc > 0 ) {
            if ( stat( *++argv, &fs ) < 0 ) {
                sprintf( msg, "cat2 %s", *argv );
                perror( msg );
                continue;
            }

            if ( ( fs.st_mode & S_IFMT ) != S_IFREG ) {
                fprintf( stderr, "cat2 %s: not a regular file\n", *argv );
                errors++;
            }
            else if ( ( fp = fopen( *argv, "r" ) ) == NULL ) {
                fprintf( stderr, "cat2: can't open %s\n", *argv );
            }
            else {
                cp2stderr( fp );
                fclose( fp );
            }
        }/*endwhile*/
    }

    return 0;
}
