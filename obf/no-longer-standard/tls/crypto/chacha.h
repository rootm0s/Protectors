/*
 * Copyright 2015-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

//OBF: Adapted from OpenSSL 1.1.0g, file internal/chacha.h:

/*
 * ChaCha20_ctr32 encrypts |len| bytes from |inp| with the given key and
 * nonce and writes the result to |out|, which may be equal to |inp|.
 * The |key| is not 32 bytes of verbatim key material though, but the
 * said material collected into 8 32-bit elements array in host byte
 * order. Same approach applies to nonce: the |counter| argument is
 * pointer to concatenated nonce and counter values collected into 4
 * 32-bit elements. This, passing crypto material collected into 32-bit
 * elements as opposite to passing verbatim byte vectors, is chosen for
 * efficiency in multi-call scenarios.
 */

/*
 * You can notice that there is no key setup procedure. Because it's
 * as trivial as collecting bytes into 32-bit elements, it's reckoned
 * that below macro is sufficient.
 */

//OBF: Adapted from OpenSSL 1.1.0g, file chacha_enc.c
//  As we're moving all the stuff to headers, to avoid potential for name collisions we have to:
//    Move whatever-possible to namespace ithare::obf::tls::
//    Add prefix ITHARE_OBF_TLS_ to all the macros (as macros don't belong to any namespace)

#ifndef ithare_obf_tls_crypto_chacha_h_included
#define ithare_obf_tls_crypto_chacha_h_included

#include "../../../src/obf.h"
#include "../../../src/obf_lib.h"

namespace ithare {
	namespace obf {
		namespace tls {

constexpr int CHACHA_KEY_SIZE = 32;
constexpr int CHACHA_CTR_SIZE = 16;
constexpr int CHACHA_BLK_SIZE = 64;

#define ITHARE_OBF_TLS_CHACHA_U8TOU32(p)  ( \
                ((unsigned int)(p)[0])     | ((unsigned int)(p)[1]<<8) | \
                ((unsigned int)(p)[2]<<16) | ((unsigned int)(p)[3]<<24)  )

/* Adapted from the public domain code by D. Bernstein from SUPERCOP. */

union chacha_buf {
    uint32_t u[16];
    uint8_t c[64];//CANNOT be used in constexpr calls; use get_byte_n() below instead in constexpr
    
    constexpr uint8_t get_byte_n( size_t idx) const {
		assert(idx <= 63);
		size_t uidx = idx / 4;
		size_t intraidx = idx % 4;
		return uint8_t(u[uidx] >> (8*intraidx));
	}
};
static_assert(sizeof(chacha_buf)==64);

# define ITHARE_OBF_TLS_ROTATE(v, n) (((v) << (n)) | ((v) >> (32 - (n))))

# define ITHARE_OBF_TLS_U32TO8_LITTLE(p, v) do { \
                                (p)[0] = (uint8_t)(v >>  0); \
                                (p)[1] = (uint8_t)(v >>  8); \
                                (p)[2] = (uint8_t)(v >> 16); \
                                (p)[3] = (uint8_t)(v >> 24); \
                                } while(0)

/* QUARTERROUND updates a, b, c, d with a ChaCha "quarter" round. */
# define ITHARE_OBF_TLS_QUARTERROUND(a,b,c,d) ( \
                x[a] += x[b], x[d] = ITHARE_OBF_TLS_ROTATE((x[d] ^ x[a]),16), \
                x[c] += x[d], x[b] = ITHARE_OBF_TLS_ROTATE((x[b] ^ x[c]),12), \
                x[a] += x[b], x[d] = ITHARE_OBF_TLS_ROTATE((x[d] ^ x[a]), 8), \
                x[c] += x[d], x[b] = ITHARE_OBF_TLS_ROTATE((x[b] ^ x[c]), 7)  )

/* chacha_core performs 20 rounds of ChaCha on the input words in
 * |input| and writes the 64 output bytes to |output|. */
ITHARE_OBF_DECLARELIBFUNC
void chacha20_core(chacha_buf *output, const uint32_t input[16])
{
	ITHARE_OBF_DBGPRINTLIBFUNCNAMEX("chacha_core");
	ITHARE_OBFLIBM1(uint32_t) x[16] = {}; ITHARE_OBF_DBGPRINTLIBX(x[0]);
	ITHARE_OBF_CALLFROMLIB(obf_copyarray)(x,input);

    for (int i = 20; i > 0; i -= 2) {
        ITHARE_OBF_TLS_QUARTERROUND(0, 4, 8, 12);
        ITHARE_OBF_TLS_QUARTERROUND(1, 5, 9, 13);
        ITHARE_OBF_TLS_QUARTERROUND(2, 6, 10, 14);
        ITHARE_OBF_TLS_QUARTERROUND(3, 7, 11, 15);
        ITHARE_OBF_TLS_QUARTERROUND(0, 5, 10, 15);
        ITHARE_OBF_TLS_QUARTERROUND(1, 6, 11, 12);
        ITHARE_OBF_TLS_QUARTERROUND(2, 7, 8, 13);
        ITHARE_OBF_TLS_QUARTERROUND(3, 4, 9, 14);
    }

	static_assert(obf_endian::native == obf_endian::little);
    if constexpr (obf_endian::native == obf_endian::little) {
        for (int i = 0; i < 16; ++i)
            output->u[i] = x[i] + input[i];
	}
    /* else ++big-endian: test
			for (int i = 0; i < 16; ++i)
            		ITHARE_OBF_TLS_U32TO8_LITTLE(output->c + 4 * i, (x[i] + input[i]));
    }*/
}

ITHARE_OBF_DECLARELIBFUNC
void ChaCha20_ctr32(unsigned char *out, const unsigned char *inp,
                    size_t len, const unsigned int key[8],
                    const unsigned int counter[4])
{
    uint32_t input[16] = {};//TODO: move initialization here

    /* sigma constant "expand 32-byte k" in little-endian encoding */
    input[0] = ((uint32_t)'e') | ((uint32_t)'x'<<8) | ((uint32_t)'p'<<16) | ((uint32_t)'a'<<24);
    input[1] = ((uint32_t)'n') | ((uint32_t)'d'<<8) | ((uint32_t)' '<<16) | ((uint32_t)'3'<<24);
    input[2] = ((uint32_t)'2') | ((uint32_t)'-'<<8) | ((uint32_t)'b'<<16) | ((uint32_t)'y'<<24);
    input[3] = ((uint32_t)'t') | ((uint32_t)'e'<<8) | ((uint32_t)' '<<16) | ((uint32_t)'k'<<24);

    input[4] = key[0];
    input[5] = key[1];
    input[6] = key[2];
    input[7] = key[3];
    input[8] = key[4];
    input[9] = key[5];
    input[10] = key[6];
    input[11] = key[7];

    input[12] = counter[0];
    input[13] = counter[1];
    input[14] = counter[2];
    input[15] = counter[3];

    chacha_buf buf = {};
    while (len > 0) {
        size_t todo = sizeof(buf);
        if (len < todo)
            todo = len;

        ITHARE_OBF_CALLFROMLIB(chacha20_core)(&buf, input);

        for (size_t i = 0; i < todo; i++) {
            if constexpr(obfflags&obf_flag_is_constexpr)//potential big-endian issues?
				out[i] = inp[i] ^ buf.get_byte_n(i);
			else
				out[i] = inp[i] ^ buf.c[i];
		}
        out += todo;
        inp += todo;
        len -= todo;

        /*
         * Advance 32-bit counter. Note that as subroutine is so to
         * say nonce-agnostic, this limited counter width doesn't
         * prevent caller from implementing wider counter. It would
         * simply take two calls split on counter overflow...
         */
        input[12]++;
    }
}

//OBF: Adapted from OpenSSL 1.1.0g, CHACHA part of file crypto/evp/e_chacha20_poly1305.c
//OBF: combined stuff into EVP_CHACHA class

class EVP_CHACHA {
	struct KEY {//former EVP_CHACHA_KEY
		struct {
		    alignas(double) /* this ensures even sizeof(EVP_CHACHA_KEY)%8==0 */
				unsigned int d[CHACHA_KEY_SIZE / 4];
		} key;
		unsigned int  counter[CHACHA_CTR_SIZE / 4];
		unsigned char buf[CHACHA_BLK_SIZE];
		unsigned int  partial_len;
	};
	KEY key;

	constexpr EVP_CHACHA( obf_private_constructor_tag, const KEY&& key_ )
	: key(std::move(key_)) {
	}
	ITHARE_OBF_DECLARELIBFUNC
	static KEY init_key(const unsigned char user_key[CHACHA_KEY_SIZE],
				const unsigned char iv[CHACHA_CTR_SIZE], int enc){
		KEY key = {};

		if (user_key)
			for (unsigned int i = 0; i < CHACHA_KEY_SIZE; i+=4) {
				key.key.d[i/4] = ITHARE_OBF_TLS_CHACHA_U8TOU32(user_key+i);
			}

		if (iv)
			for (unsigned int i = 0; i < CHACHA_CTR_SIZE; i+=4) {
				key.counter[i/4] = ITHARE_OBF_TLS_CHACHA_U8TOU32(iv+i);
			}

		key.partial_len = 0;
		return key;
	}
	ITHARE_OBF_DECLARELIBFUNC
	static void cipher( KEY& key, unsigned char *out,
			    const unsigned char *inp, size_t len)
	{
		ITHARE_OBF_DBGPRINTLIBFUNCNAMEX("EVP_CHACHA::cipher");
		ITHARE_OBFLIB(unsigned int) n = key.partial_len; ITHARE_OBF_DBGPRINTLIBX(n);
		if (n) {
			while (len && n < ITHARE_OBFILIB(CHACHA_BLK_SIZE)) {
				*out++ = *inp++ ^ key.buf[n++];
				len--;
			}
			key.partial_len = n;

			if (len == 0)
				return;

			if (n == CHACHA_BLK_SIZE) {
				key.partial_len = 0;
				key.counter[0]++;
				if (key.counter[0] == 0)
					key.counter[1]++;
			}
		}

		unsigned int rem = (unsigned int)(len % CHACHA_BLK_SIZE);
		len -= rem;
		unsigned int ctr32 = key.counter[0];//TODO: is it really unsigned int, or maybe uint32_t?
		while (len >= CHACHA_BLK_SIZE) {
			size_t blocks = len / CHACHA_BLK_SIZE;
			/*
			 * 1<<28 is just a not-so-small yet not-so-large number...
			 * Below condition is practically never met, but it has to
			 * be checked for code correctness.
			 */
			if (sizeof(size_t)>sizeof(unsigned int) && blocks>(1U<<28))
				blocks = (1U<<28);

			/*
			 * As ChaCha20_ctr32 operates on 32-bit counter, caller
			 * has to handle overflow. 'if' below detects the
			 * overflow, which is then handled by limiting the
			 * amount of blocks to the exact overflow point...
			 */
			ctr32 += (unsigned int)blocks;
			if (ctr32 < blocks) {
				blocks -= ctr32;
				ctr32 = 0;
			}
			blocks *= CHACHA_BLK_SIZE;
			ITHARE_OBF_CALLFROMLIB(ChaCha20_ctr32)(out, inp, blocks, key.key.d, key.counter);
			len -= blocks;
			inp += blocks;
			out += blocks;

			key.counter[0] = ctr32;
			if (ctr32 == 0) key.counter[1]++;
		}

		if (rem) {
			ITHARE_OBF_CALLFROMLIB(obf_zeroarray)(key.buf);
			ITHARE_OBF_CALLFROMLIB(ChaCha20_ctr32)(key.buf, key.buf, CHACHA_BLK_SIZE,
						   key.key.d, key.counter);
			for (n = 0; n < rem; n++)
				out[n] = inp[n] ^ key.buf[n];
			key.partial_len = rem;
		}
	}	

	public:
	ITHARE_OBF_DECLARELIBFUNC //cannot really invoke constructors with explicit template parameters :-(
							  //  use EVP_CHACHA::construct<...> defined below instead
	EVP_CHACHA( const unsigned char user_key[CHACHA_KEY_SIZE],
				const unsigned char iv[CHACHA_CTR_SIZE], int enc)
				: key(init_key(user_key,iv,enc)) {
	}
	ITHARE_OBF_DECLARELIBFUNC
	static EVP_CHACHA construct(const unsigned char user_key[CHACHA_KEY_SIZE],
				const unsigned char iv[CHACHA_CTR_SIZE], int enc){
		return EVP_CHACHA(obf_private_constructor_tag(),std::move(init_key(user_key,iv,enc)));
	}

	ITHARE_OBF_DECLARELIBFUNC
	void cipher(unsigned char *out,
			    const unsigned char *inp, size_t len) {
		ITHARE_OBF_CALLFROMLIB(cipher)(key, out, inp, len);
	}
	
	ITHARE_OBF_DECLARELIBFUNC_WITHEXTRA(size_t N)
	std::pair<EVP_CHACHA,ObfArrayWrapper<unsigned char,N> > constexpr_cipher(const unsigned char (&inp)[N]) const {
		KEY modifiable_key = key;
		ObfArrayWrapper<unsigned char,N> out = {}; 
		ITHARE_OBF_CALL_AS_CONSTEXPR(cipher)(modifiable_key,out.arr,inp,N);
		EVP_CHACHA new_state = EVP_CHACHA(obf_private_constructor_tag(),std::move(modifiable_key));
		return std::pair<EVP_CHACHA,ObfArrayWrapper<unsigned char,N>>(new_state,out);
	}
};



#if 0
static const EVP_CIPHER chacha20 = {
    NID_chacha20,
    1,                      /* block_size */
    CHACHA_KEY_SIZE,        /* key_len */
    CHACHA_CTR_SIZE,        /* iv_len, 128-bit counter in the context */
    EVP_CIPH_CUSTOM_IV | EVP_CIPH_ALWAYS_CALL_INIT,
    chacha_init_key,
    chacha_cipher,
    NULL,
    sizeof(EVP_CHACHA_KEY),
    NULL,
    NULL,
    NULL,
    NULL
};

const EVP_CIPHER *EVP_chacha20(void)
{
    return (&chacha20);
}
#endif

#undef ITHARE_OBF_TLS_QUARTERROUND
#undef ITHARE_OBF_TLS_U32TO8_LITTLE
#undef ITHARE_OBF_TLS_ROTATE
#undef ITHARE_OBF_TLS_CHACHA_U8TOU32

    }//namespace tls
  }//namespace obf
}//namespace ithare

#endif //#ifndef ithare_obf_tls_crypto_chacha_h_included
