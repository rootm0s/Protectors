#pragma once

#include <stdint.h>
#include <random>

namespace mut
{

class Utils
{
public:
    static bool CheckProbability( double pct )
    {
        static std::random_device rd;
        static std::uniform_real_distribution<> distrib;

        return distrib( rd ) < pct;
    }

    static int GetRandomInt( int min, int max )
    {
        static std::random_device rd;
        std::uniform_int_distribution<> distrib( min, max );

        return distrib( rd );
    }
};

}
