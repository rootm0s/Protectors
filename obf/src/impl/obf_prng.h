#ifndef ithare_obf_prng_h_included
#define ithare_obf_prng_h_included

//NOT intended to be #included directly
//  #include ../obf.h instead

#include <stdint.h>
#include "obf_common.h"
#include "obf_crypto.h"

#ifdef ITHARE_OBF_SEED

#define ITHARE_OBF_UINT64C(c) UINT64_C(c)//to avoid issues with Clang when trying to UINT64_C(MACRO)

namespace ithare {
	namespace obf {
		constexpr uint64_t obf_seed = ITHARE_OBF_UINT64C(ITHARE_OBF_SEED);
		constexpr uint64_t obf_seed2 = ITHARE_OBF_UINT64C(ITHARE_OBF_SEED2);

		constexpr uint64_t obf_const_random0[] = {
			UINT64_C(0x4d97'89b5'e76f'c505), UINT64_C(0xac1e'21fb'6594'ce31), UINT64_C(0xbc9e'7c29'054a'beb5), UINT64_C(0x418c'3b82'd2d8'a0db),
			UINT64_C(0x9220'9ecf'b9b7'cb70), UINT64_C(0xfaca'eca7'6bf3'c919), UINT64_C(0x7379'36be'7574'654a), UINT64_C(0x4205'd596'48bc'd330),
			UINT64_C(0xf2aa'35c8'c670'b1a2), UINT64_C(0xeef5'9ef8'28d8'fed2), UINT64_C(0x8d86'2109'a268'8a6b), UINT64_C(0x535d'2930'052b'7ab5),
			UINT64_C(0x7f79'42d6'cffd'21a7), UINT64_C(0x8f18'3618'68b2'c1ac), UINT64_C(0x69da'cd1c'a7fd'549a), UINT64_C(0x9345'341d'e34a'81e0),
			UINT64_C(0x5344'84d5'2a07'c80c), UINT64_C(0xee2d'ef87'6dde'1d20), UINT64_C(0x43d0'3cc3'39ae'deea), UINT64_C(0xf2f3'dbac'698d'b760)  
			
		};/*from random.org*/
		constexpr uint64_t obf_const_random1[] = {
			UINT64_C(0x30c9'c242'c935'61e2), UINT64_C(0xd529'3193'1e57'0a40), UINT64_C(0xd01d'41d1'142c'e938), UINT64_C(0x327b'2d31'2760'1f6b),
			UINT64_C(0x463b'ef56'47cb'121a), UINT64_C(0xb657'76c9'8087'3d61), UINT64_C(0x63e8'7a37'88c7'03b4), UINT64_C(0xd648'095c'c6f6'5473),
			UINT64_C(0xba02'be98'd2e6'4836), UINT64_C(0x6b2b'e8ab'f44c'2af3), UINT64_C(0x971c'4f88'0d7d'd7a5), UINT64_C(0xe728'ed94'3c0e'9724),
			UINT64_C(0xed42'd3d0'44cf'a1cc), UINT64_C(0xb333'dc8d'6f58'1f30), UINT64_C(0x2700'b0b7'ad09'32eb), UINT64_C(0xa431'c3be'c084'4f3c),
			UINT64_C(0xf11b'114c'eaab'76ed), UINT64_C(0xf387'892d'f0d6'9be2), UINT64_C(0x8078'e911'2775'9316), UINT64_C(0xce72'fb19'76fc'6ffe)   
		};/*from random.org*/

		constexpr const char* obf_normalize_fname(const char* file) {
			//normalizing __FILE__ to the bare file name (without path, which does change between debug/release)
			const char* ret = file;
			for (const char *p = file; *p; ++p)
				if (*p == '/' || *p == '\\')
					ret = p + 1;
			return ret;
		}

#ifndef ITHARE_OBF_CRYPTO_PRNG//CRYPTO PRNG slows down compile but uses 128-bit crypto-grade PRNGs 
		constexpr uint64_t obf_murmurhash2(uint64_t u, uint64_t seed) {
			//adapted to 64-bit-only input from https://sites.google.com/site/murmurhash/MurmurHash2_64.cpp
			const int len = 4;
			const uint64_t m = 0xc6a4'a793'5bd1'e995;
			const int r = 47;

			uint64_t h = seed ^ (len * m);

			//was loop in original, but we don't need loop here
			uint64_t k = u;

			k *= m;
			k ^= k >> r;
			k *= m;

			h ^= k;
			h *= m;
			//end of loop-in-original

			h ^= h >> r;
			h *= m;
			h ^= h >> r;

			return h;
		}
		constexpr uint64_t obf_ranhash(uint64_t u) {
			//ranhash from Numerical Recipes
			uint64_t v = u * UINT64_C(3935559000370003845) + UINT64_C(2691343689449507681);
			v ^= v >> 21; v ^= v << 37; v ^= v >> 4;
			v *= UINT64_C(4768777513237032717);
			v ^= v << 20; v ^= v >> 41; v ^= v << 5;
			return v;
		}
		/*constexpr uint64_t obf_swap_bits(uint64_t x) {
		uint64_t a = x & UINT64_C(0xAAAA'AAAA'AAAA'AAAA);
		uint64_t b = x & UINT64_C(0x5555'5555'5555'5555);
		return (a >> 1) | (b << 1);
		}
		constexpr uint64_t obf_swap_nibbles(uint64_t x) {
		uint64_t a = x & UINT64_C(0xF0F0'F0F0'F0F0'F0F0);
		uint64_t b = x & UINT64_C(0x0F0F'0F0F'0F0F'0F0F);
		return (a >> 4) | (b << 4);
		}*/
		constexpr uint32_t obf_random(uint64_t seed, int32_t modifier, uint32_t maxn) {
			//for maxn < 1M, bias is limited to <0.1% - more than enough for our purposes
			assert(maxn > 0);
			assert(maxn < 1'048'576);//limiting bits to 20 and bias to <0.1%
									 //  if REALLY necessary - can raise the limit at the cost of increased bias

			assert(modifier >= 0);
			assert(modifier < sizeof(obf_const_random0) / sizeof(uint64_t));//if necessary - add more random data to obf_const_random0
			uint64_t u = obf_const_random0[modifier] ^ seed;
			uint64_t v = obf_ranhash(u);
			return v % maxn;

			//uint64_t v = obf_ranhash((uint64_t(uint32_t(modifier)) << 32 )^ seed);//<<32 in hope that it will play nicely with 1st multiplication in ranhash()
			//return v % maxn;
		}
		constexpr uint32_t obf_random_uint32(uint64_t seed, int32_t modifier) {
			assert(modifier >= 0);
			assert(modifier < sizeof(obf_const_random0) / sizeof(uint64_t));//if necessary - add more random data to obf_const_random0
			uint64_t u = obf_const_random0[modifier] ^ seed;
			uint64_t v = obf_ranhash(u);
			return uint32_t(v);
		}

		constexpr uint64_t obf_new_prng(uint64_t seed, int32_t modifier) {
			assert(modifier >= 0);
			assert(modifier < sizeof(obf_const_random0) / sizeof(uint64_t));//if necessary - add more random data to obf_const_random0
			uint64_t u = obf_const_random1[modifier];
			return obf_murmurhash2(u, seed);//could use ranhash here too, but why not?
											/*
											//murmurhash
											uint64_t u = obf_murmurhash2(uint64_t(uint32_t(modifier)) << 32, seed);//<<32 in hope that it will play nicely with 1st multiplication in murmurhash2()
											//as murmurhash and ranhash use similar mechanics based on rotations-with-xors and multiplications
											//  let's add some non-linear-with-it permutations in between to avoid them cancelling out each other
											uint64_t u1 = obf_swap_bits(u);
											uint64_t u2 = obf_swap_nibbles(u1);
											//ranhash
											return obf_ranhash(u2);
											*/
		}
		constexpr uint64_t obf_init_prng(const char* file, int line, int counter) {
			uint64_t u = obf_seed ^ line;
#ifdef ITHARE_OBF_CONSISTENT_XPLATFORM_IMPLICIT_SEEDS
			const char* filename = obf_normalize_fname(file);
#else//!CONSISTENT
			u ^= counter;
			const char* filename = file;
#endif
			for (const char* p = filename; *p; ++p)//effectively djb2 by Dan Bernstein, albeit with different initializer
				u = ((u << 5) + u) + *p;
			return obf_murmurhash2(u, obf_seed2);
		}
		constexpr uint64_t obf_combined_prng(uint64_t seed, uint64_t seed2) {
			return obf_murmurhash2(seed, seed2);
		}

#define ITHARE_OBF_SEEDTPARAM uint64_t
#define ITHARE_OBF_DUMMYSEED 0
#define ITHARE_OBF_DECLAREPRNG constexpr static uint64_t /* sic! */
#define ITHARE_OBF_DECLAREPRNG_INFUNC constexpr uint64_t
#define	ITHARE_OBF_INIT_PRNG(file,line,counter) ithare::obf::obf_init_prng(file,line,counter) 
#define ITHARE_OBF_NEW_PRNG(prng,modifier) ithare::obf::obf_new_prng(prng,modifier)
#define ITHARE_OBF_COMBINED_PRNG(prng,prng2) ithare::obf::obf_combined_prng(prng,prng2)
#define ITHARE_OBF_RANDOM(prng,modifier,maxn) ithare::obf::obf_random(prng,modifier,maxn)
#define ITHARE_OBF_RANDOM_UINT32(prng,modifier) ithare::obf::obf_random_uint32(prng,modifier)
#define ITHARE_OBF_DUMMY_PRNG 0
#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		//dbgPrint helpers
		template<ITHARE_OBF_SEEDTPARAM seed>
		std::string obf_dbgPrintSeed() {
			return std::to_string(seed);
		}
#endif
#else
		template<uint64_t lo_, uint64_t hi_, int depth_>
		struct ObfSeed {
			static constexpr uint64_t lo = lo_;
			static constexpr uint64_t hi = hi_;
			static constexpr int depth = depth_;
		};
		//TODO: generate ALL the keys (for_random, for_init, and a hundred of keys for different depths) from an externally-provided-seed...
		constexpr std::array<uint32_t, 4> obf_prng_xxtea_key_for_init = {
			UINT32_C(0xf4f8'a649), UINT32_C(0xa77a'ee49), UINT32_C(0xfbdb'8ec9), UINT32_C(0x380b'2725)
		};//from random.org
		constexpr std::array<uint32_t, 4> obf_prng_xxtea_key_for_random = {
			UINT32_C(0x1fa3'669f), UINT32_C(0x7cf5'7cf9), UINT32_C(0x7df5'4887), UINT32_C(0x9c92'd8d5)
		};//from random.org
		constexpr std::array<uint32_t, 4> obf_prng_xxtea_key0 = {
			UINT32_C(0xa0d7'9b06), UINT32_C(0x29da'2659), UINT32_C(0x3b70'20ec), UINT32_C(0xa3ff'52fb)
		};//from random.org
		constexpr std::pair<uint64_t, uint64_t> obf_new_prng(uint64_t lo, uint64_t hi, int32_t modifier) {
			assert(modifier >= 0);
			assert(modifier < sizeof(obf_const_random0) / sizeof(uint64_t));//if necessary - add more random data to obf_const_random0
																			//using block cipher (currently XXTEA) in CTR mode
			assert(modifier < sizeof(obf_const_random1) / sizeof(uint64_t));//if necessary - add more random data to obf_const_random0
																			//using block cipher (currently XXTEA) in CTR mode

			uint64_t l = lo ^ obf_const_random0[modifier];
			uint64_t h = hi ^ obf_const_random1[modifier];
			std::pair<uint64_t, uint64_t> ctr_block = { l ^ obf_seed,h ^obf_seed2 };
			//TODO: use different keys for different 'depth'
			std::array<uint32_t, 4> v = obf_xxtea_encipher(ctr_block, obf_prng_xxtea_key0);

			uint64_t rlo = (uint64_t(v[1]) << 32) | uint64_t(v[0]);
			uint64_t rhi = (uint64_t(v[3]) << 32) | uint64_t(v[2]);
			return std::pair<uint64_t, uint64_t>(rlo, rhi);
		}
		constexpr uint32_t obf_random(uint64_t lo, uint64_t hi, int32_t modifier, uint32_t maxn) {
			//for maxn < 1M, bias is limited to <0.1% - more than enough for our purposes
			assert(maxn > 0);
			assert(maxn < 1'048'576);//limiting bits to 20 and bias to <0.1%
									 //  if REALLY necessary - can raise the limit at the cost of increased bias

			assert(modifier >= 0);
			assert(modifier < sizeof(obf_const_random0) / sizeof(uint64_t));//if necessary - add more random data to obf_const_random0
																			//using block cipher (currently XXTEA) in CTR mode
			assert(modifier < sizeof(obf_const_random1) / sizeof(uint64_t));//if necessary - add more random data to obf_const_random0
																			//using block cipher (currently XXTEA) in CTR mode

			uint64_t l = lo ^ obf_const_random1[modifier];//TODO: add random 2/3
			uint64_t h = hi ^ obf_const_random0[modifier];

			std::pair<uint64_t, uint64_t> ctr_block = { l ^ obf_seed,h ^obf_seed2 };
			std::array<uint32_t, 4> v = obf_xxtea_encipher(ctr_block, obf_prng_xxtea_key_for_random);
			uint64_t rlo = (uint64_t(v[1]) << 32) | uint64_t(v[0]);
			return rlo % maxn;
		}
		constexpr uint32_t obf_random_uint32(uint64_t lo, uint64_t hi, int32_t modifier) {
			assert(modifier >= 0);
			assert(modifier < sizeof(obf_const_random0) / sizeof(uint64_t));//if necessary - add more random data to obf_const_random0
																			//using block cipher (currently XXTEA) in CTR mode
			assert(modifier < sizeof(obf_const_random1) / sizeof(uint64_t));//if necessary - add more random data to obf_const_random0
																			//using block cipher (currently XXTEA) in CTR mode

			uint64_t l = lo ^ obf_const_random1[modifier];//TODO: add random 2/3
			uint64_t h = hi ^ obf_const_random0[modifier];

			std::pair<uint64_t, uint64_t> ctr_block = { l ^ obf_seed,h ^obf_seed2 };
			std::array<uint32_t, 4> v = obf_xxtea_encipher(ctr_block, obf_prng_xxtea_key_for_random);
			uint64_t rlo = (uint64_t(v[1]) << 32) | uint64_t(v[0]);
			return uint32_t(rlo);
		}
		constexpr std::pair<uint64_t, uint64_t> obf_init_prng(const char* file, int line, int counter) {
			uint64_t v0 = line;
#ifdef ITHARE_OBF_CONSISTENT_XPLATFORM_IMPLICIT_SEEDS
			const char* filename = obf_normalize_fname(file);
#else//!CONSISTENT
			v0 ^= counter;
			const char* filename = file;
#endif
			uint64_t u = obf_string_hash(filename);
			std::pair<uint64_t, uint64_t> ctr_block = { u ^ obf_seed,v0 ^obf_seed2 };
			std::array<uint32_t, 4> v = obf_xxtea_encipher(ctr_block, obf_prng_xxtea_key_for_init);
			uint64_t rlo = (uint64_t(v[1]) << 32) | uint64_t(v[0]);
			uint64_t rhi = (uint64_t(v[3]) << 32) | uint64_t(v[2]);
			return std::pair<uint64_t, uint64_t>(rlo, rhi);
		}
		constexpr std::pair<uint64_t, uint64_t> obf_combined_prng(uint64_t lo, uint64_t hi, uint64_t lo2, uint64_t hi2) {
			uint64_t l = lo ^ hi2;//why not?
			uint64_t h = hi ^ lo2;
			std::pair<uint64_t, uint64_t> ctr_block = { l ^ obf_seed,h ^obf_seed2 };
			//TODO: use different keys for different 'depth'
			std::array<uint32_t, 4> v = obf_xxtea_encipher(ctr_block, obf_prng_xxtea_key0);

			uint64_t rlo = (uint64_t(v[1]) << 32) | uint64_t(v[0]);
			uint64_t rhi = (uint64_t(v[3]) << 32) | uint64_t(v[2]);
			return std::pair<uint64_t, uint64_t>(rlo, rhi);
		}
#define ITHARE_OBF_SEEDTPARAM class /* sic! */
#define ITHARE_OBF_DUMMYSEED ithare::obf::ObfSeed<0,0,-1>
#define ITHARE_OBF_DECLAREPRNG using /* don't ask ;-) */
#define ITHARE_OBF_DECLAREPRNG_INFUNC using
#define	ITHARE_OBF_INIT_PRNG(file,line,counter) ithare::obf::ObfSeed<ithare::obf::obf_init_prng(file,line,counter).first,ithare::obf::obf_init_prng(file,line,counter).second,0>
#define ITHARE_OBF_NEW_PRNG(prng,modifier) ithare::obf::ObfSeed<ithare::obf::obf_new_prng(prng::lo,prng::hi,modifier).first,ithare::obf::obf_new_prng(prng::lo,prng::hi,modifier).second,prng::depth+1>
#define ITHARE_OBF_COMBINED_PRNG(prng,prng2) ithare::obf::ObfSeed<ithare::obf::obf_combined_prng(prng::lo,prng::hi,prng2::lo,prng2::hi).first,ithare::obf::obf_combined_prng(prng::lo,prng::hi,prng2::lo,prng2::hi).second,std::max(prng::depth,prng2::depth)+1>
#define ITHARE_OBF_RANDOM(prng,modifier,maxn) ithare::obf::obf_random(prng::lo,prng::hi,modifier,maxn)
#define ITHARE_OBF_RANDOM_UINT32(prng,modifier) ithare::obf::obf_random_uint32(prng::lo,prng::hi,modifier)
#define ITHARE_OBF_DUMMY_PRNG ObfSeed<0,0,0>
#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		//dbgPrint helpers
		template<ITHARE_OBF_SEEDTPARAM seed>
		std::string obf_dbgPrintSeed() {
			return std::string("<") + std::to_string(seed::lo) + "," + std::to_string(seed::hi) + ">";
		}
#endif

#endif//ITHARE_OBF_CRYPTO_PRNG
	}//namespace obf
}//namespace ithare

#endif//ITHARE_OBF_SEED

#endif//#ifndef ithare_obf_prng_h_included

