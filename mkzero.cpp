#include<iostream>
#include<fstream>
#include<string>
#include<array>
using namespace std::literals;

void syntax() {
  std::cout << R"(
NAME
----
mkzero -- create a binary file containing only zeroes of a specified size

SYNOPSIS
--------
mkzero [-v|-q] SIZE FILENAME
mkzero [-v|-q] FILENAME SIZE

where SIZE may have a suffix of K, M, or G.

)";
}

int main( int argc, const char* argv[] )
{
  auto filename{ ""s };
  auto filesize{ 1024ul }; // for use with single argument
  auto i=int{ 1 };
  bool verbose{true};
  for( ; i<argc and argv[i][0] == '-'; ++i ) {
    if ( "-h"s == argv[ i ] or "--help"s == argv[ i ] ) {
      syntax();
      return 0;
    } else if ( "-q"s == argv[ i ] ) {
      verbose = false;
    } else if ( "-v"s == argv[ i ] ) {
      verbose = true;
    } else {
      std::cerr << "Error: unknown option: " << argv[ i ] << '\n';
      return 1;
    }
  }
  switch ( argc - 1 ) {
    case 0: {
      std::cerr << "Error: Insufficient arguments\n";
      syntax();
      return 1;
    }
    case 1: {
      filename = argv[1];
      break;
    }
    case 2: {
      size_t pos{};
      std::string requestedSize{ argv[ 1 ] };
      filename = argv[ 2 ];
      if ( not std::isdigit( requestedSize[ 0 ] )) {
        swap( requestedSize, filename );
      }
        filesize = stoul( requestedSize, &pos );
        if( pos < requestedSize.size() ) {
          if( pos != requestedSize.size() - 1 ) {
            std::cerr << "Error: filesize of unknown format.\n";
            return 1;
          }
          else {
            pos = requestedSize.find_first_of( "kmgKMG", pos );
            if( pos == std::string::npos ) {
              std::cerr << "Error: filesize has unknown suffix.\n";
              return 1;
            }
            filesize *= (1 << 10*(1+pos%3));
          }
        }
        if ( filesize <= 0 ) {
          std::cerr << "Error: filesize must be a positive integer.\n";
          return 1;
        }
      break;
    }
    default:
      std::cerr << "Error: Too many arguments\n";
      syntax();
      return 1;
  }

  std::ofstream zf{ filename, std::ios::out | std::ios::binary };
  if ( !zf ) {
    std::cerr << "Error: Cannot open file!\n";
    return 1;
  }

  std::array<char,1024> buffer = { 0 };
  for( ; filesize >= buffer.size(); filesize -= buffer.size() ) {
    zf.write( buffer.data(), buffer.size() );
  }
  while( filesize-- ) {
    zf.write( ( char* ) "\0", 1 );
  }

  zf.close();

  if ( not zf.good() ) {
    std::cerr << "Error: Unknown error occurred at writing time!\n";
    return 1;
  }

  if( verbose ) {
    std::cout << "Created '" << filename << "' filled with " << filesize << " zeros.\n";
  }

  return 0;
}
