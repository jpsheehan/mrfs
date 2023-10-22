#include "Base64.h"

#define FILENAME 		"main"
#define FILENAME_TXT	FILENAME ".txt"
#define FILENAME_BIN	FILENAME ".bin"

int main()
{
	Base64EncodeFile( FILENAME, FILENAME_TXT );
	Base64DecodeFile( FILENAME_TXT, FILENAME_BIN );

	return 0;
}
