#include "lest.hpp"
#include "../src/obf.h"

#include "../no-longer-standard/tls/crypto/chacha.h"

#ifdef ITHARE_OBF_TEST_NO_NAMESPACE
using namespace ithare::obf;
using namespace ithare::obf::tls;
#define ITOBF
#define ITOBF_TLS
#else
#define ITOBF ithare::obf::
#define ITOBF_TLS ithare::obf::tls::
#endif

class MyException {
public:
	MyException(std::string msg)
		: message(msg) {
	}
	virtual const char* what() const {
		return message.c_str();
	}

private:
	std::string message;
};

ITHARE_OBF_NOINLINE OBF6(int64_t) factorial(OBF6(int64_t) x) {
	//DBGPRINT(x)
	if (x < 0)
		throw MyException(OBF5S("Negative argument to factorial!"));
	OBF3(int64_t) ret = 1;
	//DBGPRINT(ret)
	for (OBF3(int64_t) i = 1; i <= x; ++i) {
		//DBGPRINT(i);
		ret *= i;
	}
	return ret;
}

const lest::test spec[] = {
	CASE("types") {
		//well, at least for common 32/64-bit platforms is should stand
		using TT1 = typename ITOBF obf_integral_operator_promoconv<int8_t,unsigned short>::type;
		static_assert(std::is_same<TT1,int>::value);

		using TT1A = typename ITOBF obf_integral_operator_promoconv<unsigned short,unsigned short>::type;
		static_assert(std::is_same<TT1A,int>::value);
		
		using TT2 = typename ITOBF obf_integral_operator_promoconv<int8_t,uint64_t>::type;
		static_assert(std::is_same<TT2,uint64_t>::value);
		EXPECT((ITOBF obf_integral_operator_literal_cast_is_safe<int8_t,int,127>()==true));
		EXPECT((ITOBF obf_integral_operator_literal_cast_is_safe<int8_t,int,128>()==false));

		using TT3 = typename ITOBF obf_integral_operator_promoconv<int16_t,unsigned int>::type;
		static_assert(std::is_same<TT3,unsigned int>::value);
		EXPECT((ITOBF obf_integral_operator_literal_cast_is_safe<int32_t,unsigned int,0x7fff'ffffU>()==true));
		EXPECT((ITOBF obf_integral_operator_literal_cast_is_safe<int32_t,unsigned int,0x8000'0000U>()==false));

		using TT4 = typename ITOBF obf_integral_operator_promoconv<unsigned int,int>::type;
		static_assert(std::is_same<TT4,unsigned>::value);
		EXPECT((ITOBF obf_integral_operator_literal_cast_is_safe<TT4,int,-1>()==false));
		EXPECT((ITOBF obf_integral_operator_literal_cast_is_safe<TT4,int,0>()==true));
		EXPECT((ITOBF obf_integral_operator_literal_cast_is_safe<TT4,int,0x7fff'ffff>()==true));
	},
	CASE("factorial()") {
		auto f = factorial(17); OBF_DBGPRINT(f);
		EXPECT(f==UINT64_C(355687428096000));
		EXPECT( factorial(18) == UINT64_C(6402373705728000));
		EXPECT( factorial(19) == UINT64_C(121645100408832000));
		EXPECT( factorial(20) == UINT64_C(2432902008176640000));
		EXPECT( factorial(21) == UINT64_C(14197454024290336768));//with wrap-around(!)
	},
	CASE("chacha20(key=0...0,nonce=0...0)") {
		uint8_t user_key[ITOBF_TLS CHACHA_KEY_SIZE] = { 0 };
		uint8_t iv[ITOBF_TLS CHACHA_CTR_SIZE] = { 0 };
		ITOBF_TLS EVP_CHACHA chacha(user_key, iv, 1);
		
		uint8_t inp[16] = {0};
		uint8_t out[16] = {0};
		chacha.cipher(out, inp, 16);
		uint8_t expected_out[16] = { 0x76, 0xb8, 0xe0, 0xad, 0xa0, 0xf1, 0x3d, 0x90, 0x40, 0x5d, 0x6a, 0xe5, 0x53, 0x86, 0xbd, 0x28 };
			//from https://github.com/secworks/chacha_testvectors/blob/master/src/chacha_testvectors.txt (look for SECOND entry with "Rounds: 20" in TC1, SECOND entry corresponds to 256-bit Chacha20 which is used in TLS per RFC7539
		EXPECT(std::equal(std::begin(out), std::end(out), std::begin(expected_out), std::end(expected_out)));
	},
	CASE("chacha20(key=10...0,nonce=0...0)") {
		uint8_t user_key[ITOBF_TLS CHACHA_KEY_SIZE] = { 1, 0 };
		uint8_t iv[ITOBF_TLS CHACHA_CTR_SIZE] = { 0 };
		ITOBF_TLS EVP_CHACHA chacha(user_key, iv, 1);
		
		uint8_t inp[16] = {0};
		uint8_t out[16] = {0};
		OBF_CALL3(chacha.cipher)(out, inp, 16);
		uint8_t expected_out[16] = { 0xc5, 0xd3, 0x0a, 0x7c, 0xe1, 0xec, 0x11, 0x93, 0x78, 0xc8, 0x4f, 0x48, 0x7d, 0x77, 0x5a, 0x85 };
			//from https://github.com/secworks/chacha_testvectors/blob/master/src/chacha_testvectors.txt (look for SECOND entry with "Rounds: 20" in TC2, SECOND entry corresponds to 256-bit Chacha20 which is used in TLS per RFC7539
		EXPECT(std::equal(std::begin(out), std::end(out), std::begin(expected_out), std::end(expected_out)));
	},
	CASE("chacha20(RFC7539)") {
		uint8_t user_key[ITOBF_TLS CHACHA_KEY_SIZE] = { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f };
		uint8_t iv[ITOBF_TLS CHACHA_CTR_SIZE] = { 1,0,0,0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4a,0x00,0x00,0x00,0x00 };//1st 4 bytes are ctr
		ITOBF_TLS EVP_CHACHA chacha(user_key, iv, 1);
		
		uint8_t inp[16] = {0};
		uint8_t out[16] = {0};
		OBF_CALL3(chacha.cipher)(out, inp, 16);
		uint8_t expected_out[16] = { 0x22,0x4f,0x51,0xf3,0x40,0x1b,0xd9,0xe1,0x2f,0xde,0x27,0x6f,0xb8,0x63,0x1d,0xed };
			//from RFC7539
		EXPECT(std::equal(std::begin(out), std::end(out), std::begin(expected_out), std::end(expected_out)));
	},
	CASE("chacha20(RFC7539, fully compile-time)") {
		constexpr uint8_t user_key[ITOBF_TLS CHACHA_KEY_SIZE] = { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f };
		constexpr uint8_t iv[ITOBF_TLS CHACHA_CTR_SIZE] = { 1,0,0,0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4a,0x00,0x00,0x00,0x00 };//1st 4 bytes are ctr
		constexpr ITOBF_TLS EVP_CHACHA chacha = ITOBF_TLS EVP_CHACHA(user_key, iv, 1);
		
		constexpr uint8_t inp[16] = {0};
		constexpr std::pair<ITOBF_TLS EVP_CHACHA,ITOBF ObfArrayWrapper<unsigned char,16>> ciphered = chacha.constexpr_cipher(inp);
		constexpr ITOBF_TLS EVP_CHACHA chacha2 = ciphered.first;
		constexpr ITOBF ObfArrayWrapper<unsigned char, 16> out = ciphered.second;
		uint8_t expected_out[16] = { 0x22,0x4f,0x51,0xf3,0x40,0x1b,0xd9,0xe1,0x2f,0xde,0x27,0x6f,0xb8,0x63,0x1d,0xed };
			//from RFC7539
		EXPECT(std::equal(std::begin(out.arr), std::end(out.arr), std::begin(expected_out), std::end(expected_out)));
	},
};

int main(int argc, char** argv) {
#if !defined(ITHARE_OBF_ENABLE_AUTO_DBGPRINT) || ITHARE_OBF_ENABLE_AUTO_DBGPRINT == 2//excluding platform-specific stuff to avoid spurious changes to obftemp.txt with -DITHARE_OBF_ENABLE_AUTO_DBGPRINT
		std::cout << "sizeof(void*) == " << sizeof(void*) << std::endl;
#endif
#ifdef ITHARE_OBF_SEED
		std::cout << "ITHARE_OBF_SEED=" << std::hex << ITHARE_OBF_SEED << std::dec << std::endl;
#endif
#ifdef ITHARE_OBF_SEED2
		std::cout << "ITHARE_OBF_SEED2=" << std::hex << ITHARE_OBF_SEED2 << std::dec << std::endl;
#endif

	ITOBF obf_init();
	{
		ITOBF ObfNonBlockingCode obf_nb_guard;	
		int err = lest::run(spec,argc,argv);
		if(err)
			return err;
	}//~ObfNonBlockingCode() is called here 
	return lest::run(spec,argc,argv);//running once again, but with a chance for ~ObfNonBlockingCode() to kick in
}
