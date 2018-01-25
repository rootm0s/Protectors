//USER-MODIFIABLE FILE
//IS INCLUDED FROM impl/obf_injection.h (which is in turn included from obf.h)
//NOT TO BE INCLUDED DIRECTLY
//no "#include guard" is really necessary

//NB: at this point, we're already within ithare::obf namespace...

//BELOW is one example injection. For your own builds, you may replace it, or add your own ones along the same lines
//IF adding new ones - make sure to add them to ITHARE_OBF_USER_INJECTION_DESCRIPTOR_LIST below too

//ITHARE_OBF_FIRST_USER_INJECTION: shift+add
template<class T, class Context>
struct obf_injection_1st_user_version_descr {
	using Traits = ObfTraits<T>;
	static constexpr OBFCYCLES own_min_injection_cycles = 3;//estimate of the CPU cycles for injection; make sure to adjust according to the appetites of your injection
	static constexpr OBFCYCLES own_min_surjection_cycles = 4;//estimate of the CPU cycles for surjection; make sure to adjust according to the appetites of your injection
	static constexpr OBFCYCLES own_min_cycles = Context::context_cycles + Context::calc_cycles(own_min_injection_cycles, own_min_surjection_cycles);//magical formula, to be used for pretty much all the versions
	static constexpr ObfDescriptor descr = Traits::is_built_in ? // we want to deal with only uint8_t, uint16_t, uint32_t, and uint64_t
			ObfDescriptor(true, own_min_cycles, 100)://100 is a 'relative probability to use corresponding obf_injection_version<> in the randomly generated code'. 
													 //  For most of built-in injections, this defaults to '100', and if you want to have maximum diversity (which is usually a Good Thing(tm)) -
													 //    100 is usually a reasonably good choice
			ObfDescriptor(false,0,0);//if T is not a built-in unsigned type - ignore this injection entirely
};

template <class T, class Context, class InjectionRequirements, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
class obf_injection_version<ITHARE_OBF_FIRST_USER_INJECTION, T, Context, InjectionRequirements, seed, cycles> {
	//MUST be named use obf_injection_version<>  
	//For subsequent ones, use ITHARE_OBF_FIRST_USER_INJECTION+1, ITHARE_OBF_FIRST_USER_INJECTION+2, ... 
	using Traits = ObfTraits<T>;
	static_assert(Traits::is_bit_based);
	static_assert(Traits::nbits > 1);

public:
	static constexpr OBFCYCLES availCycles = cycles - obf_injection_1st_user_version_descr<T,Context>::own_min_cycles;//magical formula to be followed; make sure to use YOUR OWN descriptor defined above
	static_assert(availCycles >= 0);

	struct RecursiveInjectionRequirements : public InjectionRequirements {
		static constexpr size_t exclude_version = ITHARE_OBF_FIRST_USER_INJECTION;
			//preventing the same injection from being used IMMEDIATELY after this one (i.e. ITHARE_OBF_FIRST_USER_INJECTION - something-else - ITHARE_OBF_FIRST_USER_INJECTION is still possible
			//RECOMMENDED to do as shown above; an alternative is size_t(-1), but in some cases it can cause strange results 
	};

	using RecursiveInjection = obf_injection<T, Context, RecursiveInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 1), availCycles+Context::context_cycles>;
		//generating RecursiveInjection - what do we want to use after our code itself is done
		//availCycles+Context::context_cycles - magical formula to be followed, as long as you're only using one dependent injection/literal 
		//  for examples for other scenarios - see impl/obf_injection.h
		
		//On ITHARE_OBF_NEW_PRNG(seed, 1) - make sure that ALL the seeds you're using for your dependent injections/literals, 
		//  are created via ITHARE_OBF_NEW_PRNG(seed, x); 
		//  also make sure to use DIFFERENT parameter x every time
		//  Also use DIFFERENT x in ALL calls to ITHARE_OBF_RANDOM(seed,x,...) 
	
	using return_type = typename RecursiveInjection::return_type;
	
	//{ SPECIFIC to shift+add 
	constexpr static size_t shift = Traits::nbits / 2;//DON'T want to randomize number of bits here, as random stuff may work as a "signature"
	static_assert(shift < Traits::nbits);
	static constexpr T mask = (T(1) << shift) - T(1);

	template<ITHARE_OBF_SEEDTPARAM seedc>
		//seedc allows to have DIFFERENT IMPLEMENTATIONS OF THE SAME INJECTION
		//    DON'T use it to produce different results! (different results are ok for seed, NOT for seed2/seedc)
		//    See impl/obf_injection.h for examples
	ITHARE_OBF_FORCEINLINE constexpr static T local_injection(T x) {
		T y = x + ( x << shift );
		ITHARE_OBF_DBG_ASSERT_SURJECTION_LOCAL("<FIRST_USER_INJECTION>",x,y);
		return y;
	}
	template<ITHARE_OBF_SEEDTPARAM seedc>
	ITHARE_OBF_FORCEINLINE constexpr static T local_surjection(T y) {
		return y - (( y & mask ) << shift);//xx& mask was left intact in injection
	}

	static constexpr OBFINJECTIONCAPS injection_caps = 0;
	//declares lack of support for "shortcut" injected_* operations
	// for examples of "shortcut" injected_* operations (which are HIGHLY RECOMMENDED but are not strictly required) - see impl/obf_injection.h

	//} SPECIFIC to shift+add 

	template<ITHARE_OBF_SEEDTPARAM seed2>
	ITHARE_OBF_FORCEINLINE constexpr static return_type injection(T x) {
		ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed, seed2);
		T y = local_injection<ITHARE_OBF_NEW_PRNG(seedc, 1)>(x);
		return RecursiveInjection::template injection<ITHARE_OBF_NEW_PRNG(seedc, 2)>(y);
	}
	template<ITHARE_OBF_SEEDTPARAM seed2>
	ITHARE_OBF_FORCEINLINE constexpr static T surjection(return_type y) {
		ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed, seed2);
		T xx = RecursiveInjection::template surjection<ITHARE_OBF_NEW_PRNG(seedc,3)>(y);
		return local_surjection<ITHARE_OBF_NEW_PRNG(seedc, 4)>(xx);
	}

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
	static void dbgPrint(size_t offset = 0, const char* prefix = "") {
		std::cout << std::string(offset, ' ') << prefix << "obf_injection_version<FIRST_USER_INJECTION/*shift+add*/," << obf_dbgPrintT<T>() << "," << obf_dbgPrintSeed<seed>() << "," << cycles << ">: shift=" << shift << std::endl;
		RecursiveInjection::dbgPrint(offset + 1);
	}
#endif
};

// combine ALL of your injection to the following list, so impl/obf_injection.h includes them into the processing
// order MUST correspond to the order of ITHARE_OBF_FIRST_USER_INJECTION, ITHARE_OBF_FIRST_USER_INJECTION+1, etc.
#define ITHARE_OBF_USER_INJECTION_DESCRIPTOR_LIST \
	obf_injection_1st_user_version_descr<T,Context>::descr,
   
