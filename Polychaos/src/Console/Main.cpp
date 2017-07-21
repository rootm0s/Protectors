#include "../Polychaos/PEMutator.h"

#include <iostream>
#include <windows.h>

using namespace mut;

std::string ToANSI( const wchar_t* str )
{
    char buf[512];
    WideCharToMultiByte( CP_ACP, 0, str, -1, buf, sizeof(buf), NULL, NULL );
    return buf;
}

void usage()
{
    std::cout << "Usage: PolychaosConsole.exe <path_to_image> [output_path]\r\n"
              << "       path_to_image - path to the target PE image file\r\n"
              << "       output_path - resulting file, optional\r\n\r\n";
}

int main( )
{
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW( GetCommandLineW(), &argc );
    if (argc < 2)
    {
        usage();
        return 2;
    }

    try
    {
        PEMutator mutant( new MutationImpl() );

        auto out = mutant.Mutate( ToANSI( argv[1] ), argc > 2 ? ToANSI( argv[2] ) : "" );

        std::cout << "Successfully mutated. Result saved in '" << out << "'\r\n";
    }
    catch (std::exception& e)
    {
        std::cout << "Exception!\r\n" << e.what() << std::endl;
        return 1;
    }

    return 0;
}