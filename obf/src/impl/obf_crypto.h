#ifndef ithare_obf_crypto_h_included
#define ithare_obf_crypto_h_included

//NOT intended to be #included directly
//  #include ../obf.h instead

#include <stdint.h>
#include "obf_common.h"

#ifdef ITHARE_OBF_SEED

//#ifndef ITHARE_OBF_PRNG_KEY
//#define ITHARE_OBF_PRNG_KEY 0xeca204d388f4ec4e //random one from random.org
//#endif
//#ifndef ITHARE_OBF_PRNG_KEY2
//#define ITHARE_OBF_PRNG_KEY2 0x3f4414f6107ec62f //random one from random.org
//#endif

namespace ithare {
	namespace obf {
		//ALL THE ALGORITHMS IN THIS FILE WERE ADAPTED TO BE CONSTEXPR

		//XXTEA - usually enough for our obfuscation RNG purposes
		//NOT to be used for serious cryptography
		//based on code from https://en.wikipedia.org/wiki/XXTEA

#define ITHARE_OBF_XXTEA_MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

		constexpr std::array<uint32_t, 4> obf_xxtea_encipher(std::pair<uint64_t, uint64_t> v0, std::array<uint32_t, 4> key) {
			//TODO: validate implementation
			constexpr uint32_t DELTA = 0x9e3779b9;
			unsigned n = 4;
			unsigned rounds = 6 + 52 / n;
			std::array<uint32_t, 4> v{ uint32_t(v0.first), uint32_t(v0.first >> 32), uint32_t(v0.second), uint32_t(v0.second >> 32) };
			uint32_t y = 0, z = 0, sum = 0;
			unsigned p = 0, e = 0;
			sum = 0;
			z = v[n - 1];
			do {
				sum += DELTA;
				e = (sum >> 2) & 3;
				for (p = 0; p < n - 1; p++) {
					y = v[p + 1];
					z = v[p] = v[p] + ITHARE_OBF_XXTEA_MX;
				}
				y = v[0];
				z = v[n - 1] = v[n - 1] + ITHARE_OBF_XXTEA_MX;
			} while (--rounds);

			return v;
		}

#undef ITHARE_OBF_XXTEA_MX

		//TODO: add constexpr Salsa20/8 or even Chacha20; NB: using them in PRNG will increase compile times :-(, so probably should be a compile-time option

	}//namespace obf
}//namespace ithare

#endif//ITHARE_OBF_SEED

#endif//#ifndef ithare_obf_crypto_h_included
