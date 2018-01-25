#ifndef ithare_obf_injection_h_included
#define ithare_obf_injection_h_included

//NOT intended to be #included directly
//  #include ../obf.h instead

#include "obf_common.h"
#include "obf_prng.h"

//IMPORTANT: principles for cross-platform obfuscations (CRITICAL for protocol obfuscations):
//  Any platform-specific injections MUST either:
//    - have NON-platform specific calculations for probabilities across ALL the platforms 
//        - this includes ObfDescriptors in _descr, AND all the calculations within the injection itself 
//        - however, they MAY have different implementations  
//    - be disabled whenever InjectionRequirements::cross_platform_only is present
//  The same MUST stand for any 64-bit-specific injections

#ifdef ITHARE_OBF_SEED

namespace ithare {
	namespace obf {

#ifdef ITHARE_OBF_DBG_RUNTIME_CHECKS
#define ITHARE_OBF_DBG_ASSERT_SURJECTION_RECURSIVE(where,x,y) do {\
			if (surjection<seed>(y) != x) {\
				std::cout << "DBG_ASSERT_SURJECTION_RECURSIVE FAILED @" << where << ": injection(" << x << ")=" << y << " but surjection(" << y << ") = " << surjection<seed>(y) << " != " << x << std::endl; \
				dbgPrint(); \
				abort(); \
			}\
		} while(false)
#define ITHARE_OBF_DBG_ASSERT_SURJECTION_LOCAL(where,x,y) do {\
			if (local_surjection<seed>(y) != x) {\
				std::cout << "DBG_ASSERT_SURJECTION_LOCAL FAILED @" << where << ": local_injection(" << x << ")=" << y << " but local_surjection(" << y << ") = " << local_surjection<seed>(y) << " != " << x << std::endl; \
				dbgPrint(); \
				abort(); \
			}\
		} while(false)
#define ITHARE_OBF_DBG_CHECK_LITERAL(where, val, c) do {\
			if (val.value() != c) {\
				std::cout << "DBG_CHECK_LITERAL ERROR @" << where << ": " << val.value() << "!=" << c << std::endl; \
				val.dbgCheck();\
				dbgPrint(); \
				abort(); \
			}\
		}while(false)
#define ITHARE_OBF_DBG_CHECK_SHORTCUT(where,shortcut,noshortcut_expr) do {\
			auto noshort = noshortcut_expr;/* can be long*/\
			if(shortcut!=noshort) {\
				std::cout << "DBG_CHECK_SHORTCUT ERROR @" << where << ": " << shortcut << "!=" << noshort << std::endl; \
				dbgPrint();\
				abort();\
			}\
        }while(false)
#else
#define ITHARE_OBF_DBG_ASSERT_SURJECTION_RECURSIVE(where,x,y)
#define ITHARE_OBF_DBG_ASSERT_SURJECTION_LOCAL(where,x,y)
#define ITHARE_OBF_DBG_CHECK_LITERAL(where, val, c)
#define ITHARE_OBF_DBG_CHECK_SHORTCUT(where,shortcut,noshortcut_expr)
#endif//ITHARE_OBF_DBG_RUNTIME_CHECKS

		//helper constexpr functions
		template<class T, size_t N>
		constexpr T obf_compile_time_approximation(T x, std::array<T, N> xref, std::array<T, N> yref) {
			for (size_t i = 0; i < N - 1; ++i) {
				T x0 = xref[i];
				T x1 = xref[i + 1];
				if (x >= x0 && x < x1) {
					T y0 = yref[i];
					T y1 = yref[i + 1];
					return uint64_t(y0 + double(x - x0)*double(y1 - y0) / double(x1 - x0));
				}
			}
			assert(false);
			return T(0);
		}

		constexpr uint64_t obf_sqrt_very_rough_approximation(uint64_t x0) {
			std::array<uint64_t, 64> xref = {};
			std::array<uint64_t, 64> yref = {};
			for (size_t i = 1; i < 64; ++i) {
				uint64_t x = UINT64_C(1) << (i - 1);
				xref[i] = x * x;
				yref[i] = x;
			}
			return obf_compile_time_approximation(x0, xref, yref);
		}

		template<ITHARE_OBF_SEEDTPARAM seed, size_t N>
		constexpr size_t obf_random_from_list(std::array<size_t, N> weights) {
			//returns index in weights
			size_t totalWeight = 0;
			for (size_t i = 0; i < weights.size(); ++i)
				totalWeight += weights[i];
			assert(totalWeight > 0);
			assert(uint32_t(totalWeight) == totalWeight);
			size_t refWeight = ITHARE_OBF_RANDOM(seed, 1, uint32_t(totalWeight));
			assert(refWeight < totalWeight);
			for (size_t i = 0; i < weights.size(); ++i) {
				if (refWeight < weights[i])
					return i;
				refWeight -= weights[i];
			}
			assert(false);
			return size_t(-1);
		}

		//OBF_CONST_X: constants to be used throughout this particular build
		template<class T, size_t N>
		constexpr size_t obf_find_idx_in_array(std::array<T, N> arr, T value) {
			for (size_t i = 0; i < N; ++i) {
				if (arr[i] == value)
					return i;
			}
			return size_t(-1);
		}

		template<ITHARE_OBF_SEEDTPARAM seed, size_t N>
		constexpr uint8_t obf_const_x(std::array<uint8_t, N> excluded) {
			std::array<uint8_t, 6> candidates = { 3,5,7,15,25,31 };//only odd, to allow using the same set of constants in mul-by-odd injections
			std::array<size_t, 6> weights = { 100,100,100,100,100,100 };
			for (size_t i = 0; i < N; ++i) {
				size_t found = obf_find_idx_in_array(candidates, excluded[i]);
				if (found != size_t(-1))
					weights[found] = 0;
			}
			size_t idx2 = obf_random_from_list<seed>(weights);
			return candidates[idx2];
		}

		constexpr std::array<uint8_t, 0> obf_const_A_excluded = {};
		constexpr uint8_t OBF_CONST_A = obf_const_x<ITHARE_OBF_INIT_PRNG(__FILE__, __LINE__, __COUNTER__)>(obf_const_A_excluded);
		constexpr std::array<uint8_t, 1> obf_const_B_excluded = { OBF_CONST_A };
		constexpr uint8_t OBF_CONST_B = obf_const_x<ITHARE_OBF_INIT_PRNG(__FILE__, __LINE__, __COUNTER__)>(obf_const_B_excluded);
		constexpr std::array<uint8_t, 2> obf_const_C_excluded = { OBF_CONST_A, OBF_CONST_B };
		constexpr uint8_t OBF_CONST_C = obf_const_x<ITHARE_OBF_INIT_PRNG(__FILE__, __LINE__, __COUNTER__)>(obf_const_C_excluded);

		template<ITHARE_OBF_SEEDTPARAM seed, class T, size_t N>
		constexpr T obf_random_const(std::array<T, N> lst) {
			return lst[ITHARE_OBF_RANDOM(seed, 1, N)];
		}

		struct ObfDescriptor {
			bool is_recursive;
			OBFCYCLES min_cycles;
			uint32_t weight;

			constexpr ObfDescriptor(bool is_recursive_, OBFCYCLES min_cycles_, uint32_t weight_)
				: is_recursive(is_recursive_), min_cycles(min_cycles_), weight(weight_) {
			}
		};

		template<ITHARE_OBF_SEEDTPARAM seed, size_t N>
		constexpr size_t obf_random_obf_from_list(OBFCYCLES cycles, const ObfDescriptor (&descr)[N], size_t exclude_version = size_t(-1)) {
			//returns index in descr
			std::array<size_t, N> nr_weights = {};
			std::array<size_t, N> r_weights = {};
			size_t sum_r = 0;
			size_t sum_nr = 0;
			for (size_t i = 0; i < N; ++i) {
				if (i != exclude_version && cycles >= descr[i].min_cycles) {
					if (descr[i].is_recursive) {
						r_weights[i] = descr[i].weight;
						sum_r += r_weights[i];
					}
					else {
						nr_weights[i] = descr[i].weight;
						sum_nr += nr_weights[i];
					}
				}
			}
			if (sum_r)
				return obf_random_from_list<seed>(r_weights);
			else {
				assert(sum_nr > 0);
				return obf_random_from_list<seed>(nr_weights);
			}
		}

		template<ITHARE_OBF_SEEDTPARAM seed, size_t N>
		constexpr std::array<OBFCYCLES, N> obf_random_split(OBFCYCLES cycles, std::array<ObfDescriptor, N> elements) {
			//size_t totalWeight = 0;
			assert(cycles >= 0);
			OBFCYCLES mins = 0;
			for (size_t i = 0; i < N; ++i) {
				assert(elements[i].min_cycles == 0);//mins NOT to be used within calls to obf_random_split 
												  //  (it not a problem with this function, but they tend to cause WAY too much trouble in injection_version<> code
				mins += elements[i].min_cycles;
				//totalWeight += elements[i].weight;
			}
			OBFCYCLES leftovers = cycles - mins;
			assert(leftovers >= 0);
			std::array<OBFCYCLES, N> ret = {};
			size_t totalWeight = 0;
			for (size_t i = 0; i < N; ++i) {
				if (elements[i].weight > 0)
					ret[i] = OBFCYCLES(ITHARE_OBF_RANDOM(seed, int(i + 1), elements[i].weight)) + 1;//'+1' is to avoid "all-zeros" case
				else
					ret[i] = 0;
				totalWeight += ret[i];
			}
			size_t totalWeight2 = 0;
			double q = double(leftovers) / double(totalWeight);
			for (size_t i = 0; i < N; ++i) {
				ret[i] = elements[i].min_cycles + OBFCYCLES(double(ret[i]) * double(q));
				assert(ret[i] >= elements[i].min_cycles);
				totalWeight2 += ret[i];
			}
			assert(OBFCYCLES(totalWeight2) <= mins + leftovers);
			return ret;
		}

		//ObfTraits<>
		template<class T>
		struct ObfTraits;

		template<>
		struct ObfTraits<uint64_t> {
			static constexpr bool is_built_in = true;
			static std::string type_name() { return "uint64_t"; }
			using signed_type = int64_t;
			using literal_type = uint64_t;

			static constexpr bool has_half_type = true;
			using HalfT = uint32_t;
			using UintT = typename obf_larger_type<uint64_t, unsigned>::type;//UintT is a type to cast to, to avoid idiocies like uint16_t*uint16_t being promoted to signed(!) int, and then overflowing to cause UB
			static constexpr bool is_bit_based = true;
			static constexpr size_t nbits = 64;
		};

		template<>
		struct ObfTraits<uint32_t> {
			static constexpr bool is_built_in = true;
			static std::string type_name() { return "uint32_t"; }
			using signed_type = int32_t;
			using literal_type = uint32_t;

			static constexpr bool has_half_type = true;
			using HalfT = uint16_t;
			using UintT = typename obf_larger_type<uint32_t, unsigned>::type;
			static constexpr bool is_bit_based = true;
			static constexpr size_t nbits = 32;
		};

		template<>
		struct ObfTraits<uint16_t> {
			static constexpr bool is_built_in = true;
			static std::string type_name() { return "uint16_t"; }
			using signed_type = int16_t;
			using literal_type = uint16_t;

			static constexpr bool has_half_type = true;
			using HalfT = uint8_t;
			using UintT = typename obf_larger_type<uint16_t, unsigned>::type;
			static constexpr bool is_bit_based = true;
			static constexpr size_t nbits = 16;
		};

		template<>
		struct ObfTraits<uint8_t> {
			static constexpr bool is_built_in = true;
			static std::string type_name() { return "uint8_t"; }
			using signed_type = int8_t;
			using literal_type = uint8_t;

			static constexpr bool has_half_type = false;
			using UintT = typename obf_larger_type<uint8_t, unsigned>::type;
			static constexpr bool is_bit_based = true;
			static constexpr size_t nbits = 8;
		};

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
	//dbgPrint helpers
	template<class T>
	std::string obf_dbgPrintT() {
		return ObfTraits<T>::type_name();
	}

	//fwd decl
	template<size_t N>
	class ObfBitUint;

	template<class T>
	struct ObfPrintC {
		using type = T;
	};
	template<>
	struct ObfPrintC<uint8_t> {
		using type = int;
	};
	template<size_t N>
	struct ObfPrintC<ObfBitUint<N>> {
		using type = typename ObfPrintC<typename ObfBitUint<N>::T>::type;
	};

	template<class T>
	typename ObfPrintC<T>::type obf_dbgPrintC(T c) {
		return typename ObfPrintC<T>::type(c);
	}
#endif

		constexpr size_t obf_smallest_uint_size(size_t n) {
			assert(n <= 64);
			if (n <= 8)
				return 8;
			else if (n <= 16)
				return 16;
			else if (n <= 32)
				return 32;
			else {
				assert(n <= 64);
				return 64;
			}
		}

		template<class T>
		constexpr T obf_mask(size_t n) {
			assert(n <= sizeof(T) * 8);
			if (n == sizeof(T) * 8)
				return T(-1);
			else
				return (T(1) << n ) - T(1);
		}

		template<size_t N_>
		class ObfBitUint {
		public:
			static constexpr size_t N = N_;
			using T = typename obf_uint_by_size<obf_smallest_uint_size(N)>::type;
			static_assert(N <= sizeof(T) * 8);

		private:
			static constexpr T mask = obf_mask<T>(N);

		public:
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint() : val(0) {}
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint(T x) : val(x & mask) {}
			//constexpr ObfBitUint(const ObfBitUint& other) : val(other.val) {}
			//constexpr ObfBitUint(const volatile ObfBitUint& other) : val(other.val) {}
			constexpr ITHARE_OBF_FORCEINLINE operator T() const { assert((val&mask) == val); return val & mask; }

			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator -() const { return ObfBitUint(-val); }
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator *(ObfBitUint x) const { return ObfBitUint(val * x.val); }
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator +(ObfBitUint x) const { return ObfBitUint(val + x.val); }
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator -(ObfBitUint x) const { return ObfBitUint(val - x.val); }
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator %(ObfBitUint x) const { return ObfBitUint(val%x.val); }
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator /(ObfBitUint x) const { return ObfBitUint(val / x.val); }

			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator ~() const { return ObfBitUint(~val); }
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator &(ObfBitUint x) const { return ObfBitUint(val & x.val); }
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator |(ObfBitUint x) const { return ObfBitUint(val | x.val); }
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator ^(ObfBitUint x) const { return ObfBitUint(val ^ x.val); }
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator <<(size_t shift) const { return ObfBitUint(val << shift); }
			constexpr ITHARE_OBF_FORCEINLINE ObfBitUint operator >>(size_t shift) const { return ObfBitUint(val >> shift); }

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0,const char* prefix="") {
			std::cout << std::string(offset, ' ') << prefix << "ObfBitUint<" << N << ">: mask =" << obf_dbgPrintC<T>(mask) << std::endl;
		}
#endif

		private:
			T val;
		};

		template<size_t N_>
		class ObfBitSint {
		public:
			static constexpr size_t N = N_;
			using UT = typename obf_uint_by_size<obf_smallest_uint_size(N)>::type;
			using T = typename std::make_signed<UT>::type;
			static_assert(N <= sizeof(T) * 8);
			static_assert(sizeof(T) == sizeof(typename ObfBitUint<N_>::T));

		private:
			static constexpr UT mask = obf_mask<UT>(N);

		public:
			constexpr ObfBitSint() : val(0) {}
			constexpr ObfBitSint(T x) : val(UT(x)&mask) {}
			constexpr operator T() const { return T(val & mask); }

			constexpr ObfBitSint operator -() const { return ObfBitSint((~val)+1); }

		private:
			UT val;
		};

		template<size_t N>
		struct ObfTraits<ObfBitUint<N>> {
		private:
			using TT = ObfBitUint<N>;
		public:
			static constexpr bool is_built_in = false;
			static std::string type_name() {
				return std::string("ObfBitUint<") + std::to_string(N) + ">";
			}
			using signed_type = ObfBitSint<N>;
			using literal_type = typename TT::T;

			static constexpr bool has_half_type = false;
			using UintT = typename obf_larger_type<typename TT::T, unsigned>::type;
			static constexpr bool is_bit_based = true;
			static constexpr size_t nbits = N;
		};

	//forward declarations
	template<class T, class Context, class InjectionRequirements,ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection;//using function-like naming, as it is essentially a bunch-of-functions, not a real object-with-data
	template<class T, T C, class Context, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_literal_ctx;//using function-like naming, as logically it is a function-returning-constant, not a real object-with-data
	template<class T_, T_ C_, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles,OBFFLAGS flags=0>
	class obf_literal;//using function-like naming, as logically it is a function-returning-constant, not a real object-with-data

	//ObfRecursiveContext
	template<class T, class Context, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	struct ObfRecursiveContext;

	//injection-with-constant - building block both for injections and for literals
	template <size_t which, class T, class Context, class InjectionRequirements, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection_version;

	//version 0: identity
	template<class Context>
	struct obf_injection_version0_descr {
		//cannot make it a part of class obf_injection_version<0, T, C, seed, cycles>,
		//  as it would cause infinite recursion in template instantiation
		static constexpr OBFCYCLES own_min_injection_cycles = 0;
		static constexpr OBFCYCLES own_min_surjection_cycles = 0;
		static constexpr OBFCYCLES own_min_cycles = Context::context_cycles + Context::calc_cycles(own_min_injection_cycles, own_min_surjection_cycles);
		static constexpr ObfDescriptor descr = ObfDescriptor(false, own_min_cycles, 1);
	};

	template <class T, class Context, class InjectionRequirements, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection_version<0, T, Context, InjectionRequirements, seed, cycles> {
		using Traits = ObfTraits<T>;

		static constexpr OBFCYCLES availCycles = cycles - obf_injection_version0_descr<Context>::own_min_cycles;
		static_assert(availCycles >= 0);

	public:
		using return_type = T;
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injection(T x) {
			return_type ret = Context::template final_injection<seed2>(x);
			//ITHARE_OBF_DBG_ASSERT_SURJECTION_RECURSIVE("<0>", x, ret);
			return ret;
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static T surjection(return_type y) {
			return Context::template final_surjection<seed2>(y);
		}

		static constexpr OBFINJECTIONCAPS injection_caps = 0;

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0,const char* prefix="") {
			std::cout << std::string(offset, ' ') << prefix << "obf_injection_version<0/*identity*/," << obf_dbgPrintT<T>() << "," << obf_dbgPrintSeed<seed>() << "," << cycles << ">: availCycles=" << availCycles << std::endl;
			Context::dbgPrint(offset + 1);
		}
#endif
	};

	//version 1: add mod 2^n
	template<class Context>
	struct obf_injection_version1_descr {
		static constexpr OBFCYCLES own_min_injection_cycles = 1;
		static constexpr OBFCYCLES own_min_surjection_cycles = 1;
		static constexpr OBFCYCLES own_min_cycles = Context::context_cycles + Context::calc_cycles(own_min_injection_cycles, own_min_surjection_cycles);
		static constexpr ObfDescriptor descr = ObfDescriptor(true, own_min_cycles, 100);
	};

	template <class T, class Context, class InjectionRequirements, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection_version<1, T, Context, InjectionRequirements, seed, cycles> {
		using Traits = ObfTraits<T>;
	public:
		static constexpr OBFCYCLES availCycles = cycles - obf_injection_version1_descr<Context>::own_min_cycles;
		static_assert(availCycles >= 0);

		struct RecursiveInjectionRequirements : public InjectionRequirements {
			static constexpr size_t exclude_version = 1;
		};

		using RecursiveInjection = obf_injection<T, Context, RecursiveInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 1), availCycles+Context::context_cycles>;
		using return_type = typename RecursiveInjection::return_type;
		static constexpr std::array<T, 5> consts = { 0, 1, OBF_CONST_A, OBF_CONST_B, OBF_CONST_C };
		constexpr static T C = obf_random_const<ITHARE_OBF_NEW_PRNG(seed, 2)>(consts);
		static constexpr bool neg = C == 0 ? true : ITHARE_OBF_RANDOM(seed, 3, 2) == 0;
		using ST = typename Traits::signed_type;

		template<ITHARE_OBF_SEEDTPARAM seedc>
		ITHARE_OBF_FORCEINLINE constexpr static T local_injection(T x) {
			if constexpr(neg) {
				ST sx = ST(x);
				return T(-sx) + C;
			}
			else {
				return x + C;
			}
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injection(T x) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed, seed2);
			T y = local_injection<seedc>(x);
			ITHARE_OBF_DBG_ASSERT_SURJECTION_LOCAL("<1>", x, y);

			return_type ret = RecursiveInjection::template injection<seedc>(y);
			//ITHARE_OBF_DBG_ASSERT_SURJECTION_RECURSIVE("<1>/a", x, ret);
			return ret;
		}
		template<ITHARE_OBF_SEEDTPARAM seedc>
		ITHARE_OBF_FORCEINLINE constexpr static T local_surjection(T y) {
			T yy = y - C;
			if constexpr(neg) {
				ST syy = ST(yy);
				T ret = T(-syy);
				return ret;
			}
			else {
				return yy;
			}
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static T surjection(return_type y) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			T yy0 = RecursiveInjection::template surjection<seedc>(y);
			return local_surjection<seedc>(yy0);
		}

		static constexpr OBFINJECTIONCAPS injection_caps = obf_injection_has_add_mod_max_value_ex;

		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injected_add_mod_max_value_ex(return_type base,T x) {
			//effectively returns base + x (base - x if neg); sic! - no C involved
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			constexpr bool has_shortcut = RecursiveInjection::injection_caps & obf_injection_has_add_mod_max_value_ex;
			if constexpr(has_shortcut) {
				if constexpr(neg) {
					return_type ret = RecursiveInjection::template injected_add_mod_max_value_ex<seedc>(base, -ST(x));
									//mutually exclusive with all the other non-CHECK calls to injection<> => no need to randomize seedc further 
					ITHARE_OBF_DBG_CHECK_SHORTCUT("<1>/-/r",ret,RecursiveInjection::template injection<seedc>(RecursiveInjection::template surjection<seedc>(base) - x));
					ITHARE_OBF_DBG_CHECK_SHORTCUT("<1>/-/0", ret, injection<seedc>(surjection<seedc>(base) + x));
					return ret;
				}
				else {
					return_type ret = RecursiveInjection::template injected_add_mod_max_value_ex<seedc>(base, x);
					ITHARE_OBF_DBG_CHECK_SHORTCUT("<1>/+/r",ret,RecursiveInjection::template injection<seedc>(RecursiveInjection::template surjection<seedc>(base) + x));
					ITHARE_OBF_DBG_CHECK_SHORTCUT("<1>/+/0", ret, injection<seedc>(surjection<seedc>(base) + x));
					return ret;
				}
			}
			else {
				if constexpr(neg) {
					return_type ret = RecursiveInjection::template injection<seedc>(RecursiveInjection::template surjection<seedc>(base) - x);
					ITHARE_OBF_DBG_CHECK_SHORTCUT("<1>/-/1", ret, injection<seedc>(surjection<seedc>(base) + x));
					return ret;
				}
				else {
					return_type ret = RecursiveInjection::template injection<seedc>(RecursiveInjection::template surjection<seedc>(base) + x);
					ITHARE_OBF_DBG_CHECK_SHORTCUT("<1>/+/1", ret, injection<seedc>(surjection<seedc>(base)+x));
					return ret;
				}
			}
		}

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_injection_version<1/*add mod 2^N*/," << obf_dbgPrintT<T>() << "," << obf_dbgPrintSeed<seed>() << "," << cycles << ">: C=" << obf_dbgPrintC<T>(C) << " neg=" << neg << std::endl;
			RecursiveInjection::dbgPrint(offset + 1);
		}
#endif
	};

	//helper for Feistel-like: randomized_non_reversible_function 
	template<size_t which, class T, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	struct obf_randomized_non_reversible_function_version;
	//IMPORTANT: Feistel-like non-reversible functions SHOULD be short, to avoid creating code 'signatures'
	//  Therefore, currently we're NOT using any recursions here

	struct obf_randomized_non_reversible_function_version0_descr {
		static constexpr ObfDescriptor descr = ObfDescriptor(false, 0, 100);
	};

	template<class T, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	struct obf_randomized_non_reversible_function_version<0, T, seed, cycles> {
		constexpr ITHARE_OBF_FORCEINLINE T operator()(T x) {
			return x;
		}

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_randomized_non_reversible_function<0/*identity*/," << obf_dbgPrintT<T>() << "," << obf_dbgPrintSeed<seed>() << "," << cycles << ">" << std::endl;
		}
#endif		
	};

	struct obf_randomized_non_reversible_function_version1_descr {
		static constexpr ObfDescriptor descr = ObfDescriptor(true, 3, 100);
	};

	template<class T, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	struct obf_randomized_non_reversible_function_version<1,T,seed,cycles> {
		using UintT = typename ObfTraits<T>::UintT;
		constexpr ITHARE_OBF_FORCEINLINE T operator()(T x) {
			return UintT(x)*UintT(x);
		}

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_randomized_non_reversible_function<1/*x^2*/," << obf_dbgPrintT<T>() << "," << obf_dbgPrintSeed<seed>() << "," << cycles << ">" << std::endl;
		}
#endif		
	};

	struct obf_randomized_non_reversible_function_version2_descr {
		static constexpr ObfDescriptor descr = ObfDescriptor(true, 7, 100);
	};

	template<class T, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	struct obf_randomized_non_reversible_function_version<2, T, seed, cycles> {
		using ST = typename std::make_signed<T>::type;
		constexpr ITHARE_OBF_FORCEINLINE T operator()(T x) {
			ST sx = ST(x);
			return T(sx < 0 ? -sx : sx);
		}

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_randomized_non_reversible_function<2/*abs*/," << obf_dbgPrintT<T>() << "," << obf_dbgPrintSeed<seed>() << "," << cycles << ">" << std::endl;
		}
#endif		
	};

	template<size_t N>
	constexpr OBFCYCLES obf_max_min_descr(const ObfDescriptor (&descr)[N]) {
		OBFCYCLES ret = 0;
		for (size_t i = 0; i < N; ++i) {
			OBFCYCLES mn = descr[i].min_cycles;
			if (ret < mn)
				ret = mn;
		}
		return ret;
	}

	template<class T, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	struct obf_randomized_non_reversible_function {
		constexpr static ObfDescriptor descr[] = {
			obf_randomized_non_reversible_function_version0_descr::descr,
			obf_randomized_non_reversible_function_version1_descr::descr,
			obf_randomized_non_reversible_function_version2_descr::descr,
		};
		constexpr static size_t max_cycles_that_make_sense = obf_max_min_descr(descr);
		constexpr static size_t which = obf_random_obf_from_list<ITHARE_OBF_NEW_PRNG(seed, 1)>(cycles, descr);
		using FType = obf_randomized_non_reversible_function_version<which, T, seed, cycles>;
		constexpr ITHARE_OBF_FORCEINLINE T operator()(T x) {
			return FType()(x);
		}

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_randomized_non_reversible_function<" << obf_dbgPrintT<T>() << "," << obf_dbgPrintSeed<seed>() << "," << cycles << ">: which=" << which << std::endl;
			FType::dbgPrint(offset + 1);
		}
#endif		
	};

	//version 2: kinda-Feistel round
	template<class T, class Context>
	struct obf_injection_version2_descr {
		static constexpr OBFCYCLES own_min_injection_cycles = 7;
		static constexpr OBFCYCLES own_min_surjection_cycles = 7;
		static constexpr OBFCYCLES own_min_cycles = Context::context_cycles + Context::calc_cycles(own_min_injection_cycles, own_min_surjection_cycles);
		static constexpr ObfDescriptor descr =
			ObfTraits<T>::has_half_type ?
			ObfDescriptor(true, own_min_cycles, 100) :
			ObfDescriptor(false, 0, 0);
	};

	template <class T, class Context, class InjectionRequirements, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection_version<2, T, Context, InjectionRequirements, seed, cycles> {
		static_assert(std::is_integral<T>::value);
		static_assert(std::is_unsigned<T>::value);
	public:
		static constexpr OBFCYCLES availCycles = cycles - obf_injection_version2_descr<T,Context>::own_min_cycles;
		static_assert(availCycles >= 0);
		constexpr static std::array<ObfDescriptor, 2> split {
			ObfDescriptor(true,0,100),//f() 
			ObfDescriptor(true,0,100)//RecursiveInjection
		};
		static constexpr auto splitCycles = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 1)>(availCycles, split);
		static constexpr OBFCYCLES cycles_f0 = splitCycles[0];
		static constexpr OBFCYCLES cycles_rInj0 = splitCycles[1];
		static_assert(cycles_f0 + cycles_rInj0 <= availCycles);

		//doesn't make sense to use more than max_cycles_that_make_sense cycles for f...
		static constexpr OBFCYCLES max_cycles_that_make_sense = obf_randomized_non_reversible_function<T, ITHARE_OBF_DUMMY_PRNG, 0>::max_cycles_that_make_sense;
		static constexpr OBFCYCLES delta_f = cycles_f0 > max_cycles_that_make_sense ? cycles_f0 - max_cycles_that_make_sense : 0;
		static constexpr OBFCYCLES cycles_f = cycles_f0 - delta_f;
		static constexpr OBFCYCLES cycles_rInj = cycles_rInj0 + delta_f;
		static_assert(cycles_f + cycles_rInj <= availCycles);

		struct RecursiveInjectionRequirements : public InjectionRequirements {
			static constexpr size_t exclude_version = size_t(-1);
		};

		using RecursiveInjection = obf_injection<T, Context, RecursiveInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 2), cycles_rInj+ Context::context_cycles>;
		using return_type = typename RecursiveInjection::return_type;

		using halfT = typename ObfTraits<T>::HalfT;
		using FType = obf_randomized_non_reversible_function<halfT, ITHARE_OBF_NEW_PRNG(seed, 3), cycles_f>;

		constexpr static int halfTBits = sizeof(halfT) * 8;

		template<ITHARE_OBF_SEEDTPARAM seedc>
		ITHARE_OBF_FORCEINLINE constexpr static T local_injection(T x) {
			T lo = x >> halfTBits;
			//T hi = (x & mask) + f((halfT)lo);
			T hi = x + f((halfT)lo);
			return (hi << halfTBits) + lo;
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injection(T x) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed, seed2);
			T y = local_injection<seedc>(x);
			ITHARE_OBF_DBG_ASSERT_SURJECTION_LOCAL("<2>", x, y);
			return_type ret = RecursiveInjection::template injection<seedc>(y);
			//ITHARE_OBF_DBG_ASSERT_SURJECTION_RECURSIVE("<2>", x, ret);
			return ret;
		}

		template<ITHARE_OBF_SEEDTPARAM seedc>
		ITHARE_OBF_FORCEINLINE constexpr static T local_surjection(T y) {
			halfT hi = y >> halfTBits;
			T lo = y;
			//T z = (hi - f((halfT)lo)) & mask;
			halfT z = (hi - f((halfT)lo));
			return z + (lo << halfTBits);
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static T surjection(return_type y_) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed, seed2);
			T y = RecursiveInjection::template surjection<seedc>(y_);
			return local_surjection<seedc>(y);
		}

		static constexpr OBFINJECTIONCAPS injection_caps = 0;

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_injection_version<2/*kinda-Feistel*/,"<<obf_dbgPrintT<T>()<<"," << obf_dbgPrintSeed<seed>() << "," << cycles << ">:" 
				" availCycles=" << availCycles << " cycles_f=" << cycles_f << " cycles_rInj=" << cycles_rInj << std::endl;
			//auto splitCyclesRT = obf_random_split(obf_compile_time_prng(seed, 1), availCycles, split);
			//std::cout << std::string(offset, ' ') << " f():" << std::endl;
			FType::dbgPrint(offset + 1,"f():");
			//std::cout << std::string(offset, ' ') << " Recursive:" << std::endl;
			RecursiveInjection::dbgPrint(offset + 1,"Recursive:");
		}
#endif

	private:
		ITHARE_OBF_FORCEINLINE static constexpr halfT f(halfT x) {
			return FType()(x);
		}
	};

	//version 3: split-join
	template<class T,class Context>
	struct obf_injection_version3_descr {
		static constexpr OBFCYCLES own_min_injection_cycles = 7;
		static constexpr OBFCYCLES own_min_surjection_cycles = 7;
		static constexpr OBFCYCLES own_min_cycles = Context::context_cycles + Context::calc_cycles(own_min_injection_cycles, own_min_surjection_cycles);
		static constexpr ObfDescriptor descr =
			ObfTraits<T>::has_half_type ?
			ObfDescriptor(true, own_min_cycles, 100) :
			ObfDescriptor(false, 0, 0);
	};

	template <class T, class Context, class InjectionRequirements, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection_version<3, T, Context, InjectionRequirements, seed, cycles> {
		static_assert(std::is_integral<T>::value);
		static_assert(std::is_unsigned<T>::value);
	public:
		//TODO:split-join based on union
		static constexpr OBFCYCLES availCycles = cycles - obf_injection_version3_descr<T, Context>::own_min_cycles;
		static_assert(availCycles >= 0);

		using halfT = typename ObfTraits<T>::HalfT;
		constexpr static int halfTBits = sizeof(halfT) * 8;

		constexpr static std::array<ObfDescriptor, 3> split{
			ObfDescriptor(true,0,200),//RecursiveInjection
			ObfDescriptor(true,0,100),//LoInjection
			ObfDescriptor(true,0,100),//HiInjection
		};
		static constexpr auto splitCycles = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 1)>(availCycles, split);
		static constexpr OBFCYCLES cycles_rInj = splitCycles[0];
		static constexpr OBFCYCLES cycles_lo = splitCycles[1];
		static constexpr OBFCYCLES cycles_hi = splitCycles[2];
		static_assert(cycles_rInj + cycles_lo + cycles_hi <= availCycles);

		struct RecursiveInjectionRequirements : public InjectionRequirements {
			static constexpr size_t exclude_version = size_t(-1);
		};
		using RecursiveInjection = obf_injection<T, Context, RecursiveInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 2), cycles_rInj+ Context::context_cycles>;
		using return_type = typename RecursiveInjection::return_type;

		struct LoHiInjectionRequirements : public InjectionRequirements {
			static constexpr size_t exclude_version = size_t(-1);
			static constexpr bool only_bijections = true;
		};

		constexpr static std::array<ObfDescriptor, 2> splitLo {
			ObfDescriptor(true,0,100),//Context
			ObfDescriptor(true,0,100)//Injection
		};
		static constexpr auto splitCyclesLo = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 2)>(cycles_lo, splitLo);
		static constexpr OBFCYCLES cycles_loCtx = splitCyclesLo[0];
		static constexpr OBFCYCLES cycles_loInj = splitCyclesLo[1];
		static_assert(cycles_loCtx + cycles_loInj <= cycles_lo);
		using LoContext = typename ObfRecursiveContext < halfT, Context, ITHARE_OBF_NEW_PRNG(seed, 3), cycles_loCtx>::intermediate_context_type;
		using LoInjection = obf_injection<halfT, LoContext, LoHiInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 4), cycles_loInj+LoContext::context_cycles>;
		static_assert(sizeof(typename LoInjection::return_type) == sizeof(halfT));//bijections ONLY

		constexpr static std::array<ObfDescriptor, 2> splitHi{
			ObfDescriptor(true,0,100),//Context
			ObfDescriptor(true,0,100)//Injection
		};
		static constexpr auto splitCyclesHi = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 5)>(cycles_hi, splitHi);
		static constexpr OBFCYCLES cycles_hiCtx = splitCyclesHi[0];
		static constexpr OBFCYCLES cycles_hiInj = splitCyclesHi[1];
		static_assert(cycles_hiCtx + cycles_hiInj <= cycles_hi);
		using HiContext = typename ObfRecursiveContext<halfT, Context, ITHARE_OBF_NEW_PRNG(seed, 6), cycles_hiCtx>::intermediate_context_type;
		using HiInjection = obf_injection<halfT, HiContext, LoHiInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 7), cycles_hiInj+HiContext::context_cycles>;
		static_assert(sizeof(typename HiInjection::return_type) == sizeof(halfT));//bijections ONLY

		template<ITHARE_OBF_SEEDTPARAM seedc>
		ITHARE_OBF_FORCEINLINE constexpr static T local_injection(T x) {
			halfT lo = x >> halfTBits;
			typename LoInjection::return_type lo1 = LoInjection::template injection<ITHARE_OBF_NEW_PRNG(seedc,1)>(lo);
			lo = halfT(lo1);// *reinterpret_cast<halfT*>(&lo1);//relies on static_assert(sizeof(return_type)==sizeof(halfT)) above
			halfT hi = (halfT)x;
			typename HiInjection::return_type hi1 = HiInjection::template injection<ITHARE_OBF_NEW_PRNG(seedc,2)>(hi);
			hi = halfT(hi1);// *reinterpret_cast<halfT*>(&hi1);//relies on static_assert(sizeof(return_type)==sizeof(halfT)) above
			return (T(hi) << halfTBits) + T(lo);
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injection(T x) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed, seed2);
			T y = local_injection<seedc>(x);
			ITHARE_OBF_DBG_ASSERT_SURJECTION_LOCAL("<3>", x, y);
			return_type ret = RecursiveInjection::template injection<ITHARE_OBF_NEW_PRNG(seedc, 3)>(y);
			//ITHARE_OBF_DBG_ASSERT_SURJECTION_RECURSIVE("<3>", x, ret);
			return ret;
		}

		template<ITHARE_OBF_SEEDTPARAM seedc>
		ITHARE_OBF_FORCEINLINE constexpr static T local_surjection(T y) {
			halfT hi0 = y >> halfTBits;
			halfT lo0 = (halfT)y;
			halfT hi = HiInjection::template surjection<ITHARE_OBF_NEW_PRNG(seedc, 5)>(/* *reinterpret_cast<typename HiInjection::return_type*>(&hi0)*/typename HiInjection::return_type(hi0));//relies on static_assert(sizeof(return_type)==sizeof(halfT)) above
			halfT lo = LoInjection::template surjection<ITHARE_OBF_NEW_PRNG(seedc, 6)>(/**reinterpret_cast<typename LoInjection::return_type*>(&lo0)*/ typename LoInjection::return_type(lo0));//relies on static_assert(sizeof(return_type)==sizeof(halfT)) above
			return T(hi) + (T(lo) << halfTBits);
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static T surjection(return_type y_) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed, seed2);
			T y = RecursiveInjection::template surjection<ITHARE_OBF_NEW_PRNG(seedc, 4)>(y_);
			return local_surjection<seedc>(y);
		}

		static constexpr OBFINJECTIONCAPS injection_caps = 0;

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_injection_version<3/*split-join*/,"<<obf_dbgPrintT<T>()<<"," << obf_dbgPrintSeed<seed>() << "," << cycles << ">" << std::endl;
			//std::cout << std::string(offset, ' ') << " Lo:" << std::endl;
			LoInjection::dbgPrint(offset + 1,"Lo:");
			//std::cout << std::string(offset, ' ') << " Hi:" << std::endl;
			HiInjection::dbgPrint(offset + 1,"Hi:");
			//std::cout << std::string(offset, ' ') << " Recursive:" << std::endl;
			RecursiveInjection::dbgPrint(offset + 1,"Recursive:");
		}
#endif
	};
	
	//version 4: multiply by odd
	template< class T >
	constexpr T obf_mul_inverse_mod2n(T num) {//extended GCD, intended to be used in compile-time only
											  //implementation by Dmytro Ivanchykhin
		assert(num & T(1));
		T num0 = num;
		T x = 0, lastx = 1, y = 1, lasty = 0;
		T q=0, temp1=0, temp2=0, temp3=0;
		T mod = 0;

		// zero step: do some tricks to avoid overflowing
		// note that initially mod is power of 2 that does not fit to T
		if (num == T(mod - T(1)))
			return num;
		q = T((T(mod - num)) / num) + T(1);
		temp1 = (T(T(T(mod - T(2))) % num) + T(2)) % num;
		mod = num;
		num = temp1;

		temp2 = x;
		x = lastx - T(q * x);
		lastx = temp2;

		temp3 = y;
		y = lasty - T(q * y);
		lasty = temp3;

		while (num != 0) {
			q = mod / num;
			temp1 = mod % num;
			mod = num;
			num = temp1;

			temp2 = x;
			x = lastx - T(q * x);
			lastx = temp2;

			temp3 = y;
			y = lasty - T(q * y);
			lasty = temp3;
		}
		assert(T(num0*lasty) == T(1));
		return lasty;
	}

	template<class Context>
	struct obf_injection_version4_descr {
		static constexpr OBFCYCLES own_min_injection_cycles = 3 + Context::literal_cycles;
		static constexpr OBFCYCLES own_min_surjection_cycles = 3;
		static constexpr OBFCYCLES own_min_cycles = Context::context_cycles + Context::calc_cycles(own_min_injection_cycles, own_min_surjection_cycles);
		static constexpr ObfDescriptor descr = ObfDescriptor(true, own_min_cycles, 100);
	};

	template <class T, class Context, class InjectionRequirements, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection_version<4, T, Context, InjectionRequirements, seed, cycles> {
		using Traits = ObfTraits<T>;
		static constexpr OBFCYCLES availCycles = cycles - obf_injection_version4_descr<Context>::own_min_cycles;
		static_assert(availCycles >= 0);

		struct RecursiveInjectionRequirements : public InjectionRequirements {
			static constexpr size_t exclude_version = 4;
		};

	public:
		using RecursiveInjection = obf_injection<T, Context, RecursiveInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 1), availCycles+Context::context_cycles>;
		using return_type = typename RecursiveInjection::return_type;
		//constexpr static T C = (T)(obf_gen_const<T>(obf_compile_time_prng(seed, 2)) | 1);
		static constexpr std::array<T, 3> consts = { OBF_CONST_A,OBF_CONST_B,OBF_CONST_C };
		constexpr static T C = obf_random_const<ITHARE_OBF_NEW_PRNG(seed, 2)>(consts);
		static_assert((C & T(1)) == 1);
		constexpr static T CINV0 = obf_mul_inverse_mod2n(C);
		static_assert((T)(C*CINV0) == (T)1);
		constexpr static typename Traits::literal_type CINV = CINV0;

		using literal = typename Context::template literal<typename Traits::literal_type, CINV, ITHARE_OBF_NEW_PRNG(seed, 3)>::type;
			//using CINV in injection to hide literals a bit better...

		template<ITHARE_OBF_SEEDTPARAM seedc>
		ITHARE_OBF_FORCEINLINE constexpr static T local_injection(T x) {
			auto lit = literal();
			ITHARE_OBF_DBG_CHECK_LITERAL("<4>", lit, CINV0);
			T y = typename Traits::UintT(x) * typename Traits::UintT(lit.value());
			return y;
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injection(T x) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed, seed2);
			T y = local_injection<seedc>(x);
			ITHARE_OBF_DBG_ASSERT_SURJECTION_LOCAL("<4>", x, y);
			return_type ret = RecursiveInjection::template injection<seedc>(y);
			//ITHARE_OBF_DBG_ASSERT_SURJECTION_RECURSIVE("<4>", x, ret);
			return ret;
		}

		template<ITHARE_OBF_SEEDTPARAM seedc>
		ITHARE_OBF_FORCEINLINE constexpr static T local_surjection(T y) {
			return y * C;
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static T surjection(return_type y) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed, seed2);
			T x = RecursiveInjection::template surjection<seedc>(y);
			return local_surjection<seedc>(x);
		}

		static constexpr OBFINJECTIONCAPS injection_caps = obf_injection_has_add_mod_max_value_ex;
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injected_add_mod_max_value_ex(return_type base, T x) {
			//effectively returns base + x*CINV0
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			if constexpr(RecursiveInjection::injection_caps & obf_injection_has_add_mod_max_value_ex) {
				auto lit = literal();
				ITHARE_OBF_DBG_CHECK_LITERAL("<4>/0", lit, CINV0);
				return_type ret = RecursiveInjection::template injected_add_mod_max_value_ex<seedc>(base, x*lit.value());
				ITHARE_OBF_DBG_CHECK_SHORTCUT("<4>/r", ret, RecursiveInjection::template injection<seedc>(RecursiveInjection::template surjection<seedc>(base) + CINV0*x));
				ITHARE_OBF_DBG_CHECK_SHORTCUT("<4>/0", ret, injection<seedc>(surjection<seedc>(base) + x));
				return ret;
			}
			else {
				auto lit = literal();
				ITHARE_OBF_DBG_CHECK_LITERAL("<4>/1", lit, CINV0);
				return_type ret = RecursiveInjection::template injection<seedc>(RecursiveInjection::template surjection<seedc>(base) + lit.value()*x);
				ITHARE_OBF_DBG_CHECK_SHORTCUT("<4>/1", ret, injection<seedc>(surjection<seedc>(base) + x));
				return ret;
			}
		}

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_injection_version<4/*mul odd mod 2^N*/,"<<obf_dbgPrintT<T>()<<"," << obf_dbgPrintSeed<seed>() << "," << cycles << ">: C=" << obf_dbgPrintC<T>(C) << " CINV=" << obf_dbgPrintC<T>(CINV) << std::endl;
			//std::cout << std::string(offset, ' ') << " literal:" << std::endl;
			literal::dbgPrint(offset + 1,"literal:");
			//std::cout << std::string(offset, ' ') << " Recursive:" << std::endl;
			RecursiveInjection::dbgPrint(offset + 1,"Recursive:");
		}
#endif
	};

	//version 5: split (w/o join)
	template<class T, class Context>
	struct obf_injection_version5_descr {
		static constexpr OBFCYCLES own_min_injection_cycles = 3;
		static constexpr OBFCYCLES own_min_surjection_cycles = 3;
		static constexpr OBFCYCLES own_min_cycles = 2*Context::context_cycles /* have to allocate context_cycles for BOTH branches */ + Context::calc_cycles(own_min_injection_cycles, own_min_surjection_cycles);
		static constexpr ObfDescriptor descr =
			ObfTraits<T>::has_half_type ?
			ObfDescriptor(true, own_min_cycles, 100) :
			ObfDescriptor(false, 0, 0);
	};

	template <class T, class Context, class InjectionRequirements, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection_version<5, T, Context, InjectionRequirements, seed, cycles> {
		static_assert(std::is_integral<T>::value);
		static_assert(std::is_unsigned<T>::value);
	public:
		using halfT = typename ObfTraits<T>::HalfT;
		constexpr static int halfTBits = sizeof(halfT) * 8;

		struct RecursiveInjectionRequirements : public InjectionRequirements {
			static constexpr size_t exclude_version = size_t(-1);
		};

		static constexpr OBFCYCLES availCycles = cycles - obf_injection_version5_descr<T,Context>::own_min_cycles;
		static_assert(availCycles >= 0);

		constexpr static std::array<ObfDescriptor, 2> split{
			ObfDescriptor(true,0,100),//LoInjection
			ObfDescriptor(true,0,100),//HiInjection
		};
		static constexpr auto splitCycles = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 1)>(availCycles, split);
		static constexpr OBFCYCLES cycles_lo = splitCycles[0];
		static constexpr OBFCYCLES cycles_hi = splitCycles[1];
		static_assert(cycles_lo + cycles_hi <= availCycles);

		constexpr static std::array<ObfDescriptor, 2> splitLo{
			ObfDescriptor(true,0,100),//Context
			ObfDescriptor(true,0,100)//Injection
		};
		static constexpr auto splitCyclesLo = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 2)>(cycles_lo, splitLo);
		static constexpr OBFCYCLES cycles_loCtx = splitCyclesLo[0];
		static constexpr OBFCYCLES cycles_loInj = splitCyclesLo[1];
		static_assert(cycles_loCtx + cycles_loInj <= cycles_lo);
		using RecursiveLoContext = typename ObfRecursiveContext<halfT, Context, ITHARE_OBF_NEW_PRNG(seed, 3), cycles_loCtx+Context::context_cycles>::recursive_context_type;
		using RecursiveInjectionLo = obf_injection<halfT, RecursiveLoContext, RecursiveInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 4), cycles_loInj+ RecursiveLoContext::context_cycles>;

		constexpr static std::array<ObfDescriptor, 2> splitHi{
			ObfDescriptor(true,0,100),//Context
			ObfDescriptor(true,0,100)//Injection
		};
		static constexpr auto splitCyclesHi = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 5)>(cycles_hi, splitHi);
		static constexpr OBFCYCLES cycles_hiCtx = splitCyclesHi[0];
		static constexpr OBFCYCLES cycles_hiInj = splitCyclesHi[1];
		static_assert(cycles_hiCtx + cycles_hiInj <= cycles_hi);
		using RecursiveHiContext = typename ObfRecursiveContext<halfT, Context, ITHARE_OBF_NEW_PRNG(seed, 6), cycles_hiCtx+Context::context_cycles>::recursive_context_type;
		using RecursiveInjectionHi = obf_injection < halfT, RecursiveHiContext, RecursiveInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 7), cycles_hiInj+ RecursiveHiContext::context_cycles> ;

		struct return_type {
			typename RecursiveInjectionLo::return_type lo;
			typename RecursiveInjectionHi::return_type hi;

			constexpr return_type(typename RecursiveInjectionLo::return_type lo_, typename RecursiveInjectionHi::return_type hi_)
				: lo(lo_), hi(hi_) {
			}
			constexpr return_type(T x) 
			: lo(halfT(x)), hi(halfT(x>>halfTBits)){
			}
			constexpr operator T() {
				halfT lo1 = halfT(lo);
				halfT hi1 = halfT(hi);
				return ( T(hi1) << halfTBits ) + T(lo1);
			}
		};
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injection(T x) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			return_type ret{ RecursiveInjectionLo::template injection<ITHARE_OBF_NEW_PRNG(seedc,1)>((halfT)x), RecursiveInjectionHi::template injection<ITHARE_OBF_NEW_PRNG(seedc,2)>(x >> halfTBits) };
			ITHARE_OBF_DBG_ASSERT_SURJECTION_RECURSIVE("<5>", x, ret);//sic! - _ASSERTION_RECURSIVE for version<5,...> (moving to local_injection would be too cumbersome)
			return ret;
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static T surjection(return_type y_) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			halfT hi = RecursiveInjectionHi::template surjection<ITHARE_OBF_NEW_PRNG(seedc,3)>(y_.hi);
			halfT lo = RecursiveInjectionLo::template surjection<ITHARE_OBF_NEW_PRNG(seedc,4)>(y_.lo);
			return (T)lo + ((T)hi << halfTBits);
		}

		static constexpr OBFINJECTIONCAPS injection_caps = 0;

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_injection_version<5/*split*/,"<<obf_dbgPrintT<T>()<<"," << obf_dbgPrintSeed<seed>() << "," << cycles << ">" << std::endl;
			//std::cout << std::string(offset, ' ') << " Lo:" << std::endl;
			RecursiveInjectionLo::dbgPrint(offset + 1,"Lo:");
			//std::cout << std::string(offset, ' ') << " Hi:" << std::endl;
			RecursiveInjectionHi::dbgPrint(offset + 1,"Hi:");
		}
#endif
	};

	//version 6: injection over lower half /*CHEAP!*/
	template<class T, class Context>
	struct obf_injection_version6_descr {
		static constexpr OBFCYCLES own_min_injection_cycles = 3;
		static constexpr OBFCYCLES own_min_surjection_cycles = 3;
		static constexpr OBFCYCLES own_min_cycles = Context::context_cycles + Context::calc_cycles(own_min_injection_cycles, own_min_surjection_cycles);
		static constexpr ObfDescriptor descr =
			ObfTraits<T>::has_half_type ?
			ObfDescriptor(true, own_min_cycles, 100/*it's cheap, but doesn't obfuscate the whole thing well => let's use it mostly for lower-cycle stuff*/) :
			ObfDescriptor(false, 0, 0);
	};

	template <class T, class Context, class InjectionRequirements, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection_version<6, T, Context, InjectionRequirements, seed, cycles> {
		static_assert(std::is_integral<T>::value);
		static_assert(std::is_unsigned<T>::value);
	public:
		static constexpr OBFCYCLES availCycles = cycles - obf_injection_version6_descr<T,Context>::own_min_cycles;
		static_assert(availCycles >= 0);

		struct RecursiveInjectionRequirements : public InjectionRequirements {
			static constexpr size_t exclude_version = 6;
		};
		using halfT = typename ObfTraits<T>::HalfT;

		constexpr static std::array<ObfDescriptor, 2> split{
			ObfDescriptor(true,0,200),//RecursiveInjection
			ObfDescriptor(true,0,100),//LoInjection
		};
		static constexpr auto splitCycles = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 1)>(availCycles, split);
		static constexpr OBFCYCLES cycles_rInj = splitCycles[0];
		static constexpr OBFCYCLES cycles_lo = splitCycles[1];
		static_assert(cycles_rInj + cycles_lo <= availCycles);

		using RecursiveInjection = obf_injection<T, Context, RecursiveInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 2), cycles_rInj + Context::context_cycles>;
		using return_type = typename RecursiveInjection::return_type;

	public:
		struct LoInjectionRequirements : public InjectionRequirements {
			static constexpr size_t exclude_version = size_t(-1);
			static constexpr bool only_bijections = true;
		};

		constexpr static std::array<ObfDescriptor, 2> splitLo{
			ObfDescriptor(true,0,100),//Context
			ObfDescriptor(true,0,100)//Injection
		};
		static constexpr auto splitCyclesLo = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 3)>(cycles_lo, splitLo);
		static constexpr OBFCYCLES cycles_loCtx = splitCyclesLo[0];
		static constexpr OBFCYCLES cycles_loInj = splitCyclesLo[1];
		static_assert(cycles_loCtx + cycles_loInj <= cycles_lo);
		using LoContext = typename ObfRecursiveContext < halfT, Context, ITHARE_OBF_NEW_PRNG(seed, 4), cycles_loCtx>::intermediate_context_type;
		using LoInjection = obf_injection<halfT, LoContext,  LoInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 5), cycles_loInj + LoContext::context_cycles>;
		static_assert(sizeof(typename LoInjection::return_type) == sizeof(halfT));//only_bijections

		template<ITHARE_OBF_SEEDTPARAM seedc>
		ITHARE_OBF_FORCEINLINE constexpr static T local_injection(T x) {
			halfT lo0 = halfT(x);
			typename LoInjection::return_type lo1 = LoInjection::template injection<ITHARE_OBF_NEW_PRNG(seedc, 1)>(lo0);
			//halfT lo = *reinterpret_cast<halfT*>(&lo1);//relies on static_assert(sizeof(return_type)==sizeof(halfT)) above
			halfT lo = halfT(lo1);
			T y = x - T(lo0) + lo;
			return y;
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injection(T x) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			T y = local_injection<seedc>(x);
			ITHARE_OBF_DBG_ASSERT_SURJECTION_LOCAL("<6>", x, y);
			return_type ret = RecursiveInjection::template injection<ITHARE_OBF_NEW_PRNG(seedc,2)>(y);
			//ITHARE_OBF_DBG_ASSERT_SURJECTION_RECURSIVE("<6>", x, ret);
			return ret;
		}

		template<ITHARE_OBF_SEEDTPARAM seedc>
		ITHARE_OBF_FORCEINLINE constexpr static T local_surjection(T y) {
			halfT lo0 = halfT(y);
			halfT lo = LoInjection::template surjection<ITHARE_OBF_NEW_PRNG(seedc,4)>(/* *reinterpret_cast<typename LoInjection::return_type*>(&lo0)*/ typename LoInjection::return_type(lo0));//relies on static_assert(sizeof(return_type)==sizeof(halfT)) above
			return y - T(lo0) + lo;
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static T surjection(return_type yy) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed, seed2);
			T y = RecursiveInjection::template surjection<ITHARE_OBF_NEW_PRNG(seedc, 3)>(yy);
			return local_surjection<seedc>(y);
		}

		static constexpr OBFINJECTIONCAPS injection_caps = 0;

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_injection_version<6/*injection(halfT)*/," << obf_dbgPrintT<T>() << "," << obf_dbgPrintSeed<seed>() << "," << cycles << ">" << std::endl;
			LoInjection::dbgPrint(offset + 1, "Lo:");
			RecursiveInjection::dbgPrint(offset + 1, "Recursive:");
		}
#endif
	};

	//version 7: split into two ObfBitUints (w/o join)
	template<class T, class Context, class InjectionRequirements>
	struct obf_injection_version7_descr {
		static constexpr OBFCYCLES own_min_injection_cycles = 15;
		static constexpr OBFCYCLES own_min_surjection_cycles = 15;
		static constexpr OBFCYCLES own_min_cycles = 2 * Context::context_cycles /* have to allocate context_cycles for BOTH branches */ + Context::calc_cycles(own_min_injection_cycles, own_min_surjection_cycles);
		static constexpr ObfDescriptor descr =
			!InjectionRequirements::only_bijections && !InjectionRequirements::no_substrate_size_increase && ObfTraits<T>::nbits >= 2 ?
			ObfDescriptor(true, own_min_cycles, 100)
			: 
			ObfDescriptor(false, 0, 0);
	};

	template <class T, class Context, class InjectionRequirements, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection_version<7, T, Context, InjectionRequirements, seed, cycles> {
		using Traits = ObfTraits<T>;
	public:
		static constexpr size_t loBits = ITHARE_OBF_RANDOM(seed, 1, Traits::nbits-1) + 1;
		static_assert(loBits > 0);
		static_assert(loBits < Traits::nbits);
		static constexpr size_t hiBits = Traits::nbits - loBits;
		using TypeLo = ObfBitUint<loBits>;
		using TypeHi = ObfBitUint<hiBits>;

		struct RecursiveInjectionRequirements : public InjectionRequirements {
			static constexpr size_t exclude_version = size_t(-1);
			static constexpr bool no_substrate_size_increase = true;//not a strict requirement, but we don't want to grow infinitely 
		};

		static constexpr OBFCYCLES availCycles = cycles - obf_injection_version7_descr<T, Context,InjectionRequirements>::own_min_cycles;
		static_assert(availCycles >= 0);

		constexpr static std::array<ObfDescriptor, 2> split{
			ObfDescriptor(true,0,100),//LoInjection
			ObfDescriptor(true,0,100),//HiInjection
		};
		static constexpr auto splitCycles = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 2)>(availCycles, split);
		static constexpr OBFCYCLES cycles_lo = splitCycles[0];
		static constexpr OBFCYCLES cycles_hi = splitCycles[1];
		static_assert(cycles_lo + cycles_hi <= availCycles);

		constexpr static std::array<ObfDescriptor, 2> splitLo{
			ObfDescriptor(true,0,100),//Context
			ObfDescriptor(true,0,100)//Injection
		};
		static constexpr auto splitCyclesLo = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 2)>(cycles_lo, splitLo);
		static constexpr OBFCYCLES cycles_loCtx = splitCyclesLo[0];
		static constexpr OBFCYCLES cycles_loInj = splitCyclesLo[1];
		static_assert(cycles_loCtx + cycles_loInj <= cycles_lo);
		using RecursiveLoContext = typename ObfRecursiveContext<TypeLo, Context, ITHARE_OBF_NEW_PRNG(seed, 3), cycles_loCtx + Context::context_cycles>::recursive_context_type;
		using RecursiveInjectionLo = obf_injection<TypeLo, RecursiveLoContext, RecursiveInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 4), cycles_loInj + RecursiveLoContext::context_cycles>;

		constexpr static std::array<ObfDescriptor, 2> splitHi{
			ObfDescriptor(true,0,100),//Context
			ObfDescriptor(true,0,100)//Injection
		};
		static constexpr auto splitCyclesHi = obf_random_split<ITHARE_OBF_NEW_PRNG(seed, 5)>(cycles_hi, splitHi);
		static constexpr OBFCYCLES cycles_hiCtx = splitCyclesHi[0];
		static constexpr OBFCYCLES cycles_hiInj = splitCyclesHi[1];
		static_assert(cycles_hiCtx + cycles_hiInj <= cycles_hi);
		using RecursiveHiContext = typename ObfRecursiveContext<TypeHi, Context, ITHARE_OBF_NEW_PRNG(seed, 6), cycles_hiCtx + Context::context_cycles>::recursive_context_type;
		using RecursiveInjectionHi = obf_injection <TypeHi, RecursiveHiContext, RecursiveInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 7), cycles_hiInj + RecursiveHiContext::context_cycles >;

		struct return_type {
			typename RecursiveInjectionLo::return_type lo;
			typename RecursiveInjectionHi::return_type hi;

			constexpr return_type(typename RecursiveInjectionLo::return_type lo_, typename RecursiveInjectionHi::return_type hi_)
				: lo(lo_), hi(hi_) {
			}
			constexpr return_type(T x)
				: lo(TypeLo(x)), hi(TypeHi(x >> loBits)) {
			}
			constexpr operator T() {
				TypeLo lo1 = TypeLo(lo);
				TypeHi hi1 = TypeHi(hi);
				return (T(hi1) << loBits) + T(lo1);
			}
		};

		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injection(T x) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			TypeLo lo = TypeLo(typename TypeLo::T(x));
			TypeHi hi = TypeHi(typename TypeHi::T(x >> loBits));
			return_type ret{ RecursiveInjectionLo::template injection<ITHARE_OBF_NEW_PRNG(seedc,1)>(lo),
				RecursiveInjectionHi::template injection<ITHARE_OBF_NEW_PRNG(seedc,2)>(hi) };
			ITHARE_OBF_DBG_ASSERT_SURJECTION_RECURSIVE("<7>", x, ret);//sic! - _ASSERTION_RECURSIVE for version<7,...> (moving to local_injection would be too cumbersome)
			return ret;
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static T surjection(return_type y_) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			TypeHi hi = RecursiveInjectionHi::template surjection<ITHARE_OBF_NEW_PRNG(seedc,3)>(y_.hi);
			TypeLo lo = RecursiveInjectionLo::template surjection<ITHARE_OBF_NEW_PRNG(seedc,4)>(y_.lo);

			T ret = T(lo) + T(T(hi) << loBits);
			return ret;
		}

		static constexpr OBFINJECTIONCAPS injection_caps = 0;

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_injection_version<7/*split into ObfBitUint<>*/," << obf_dbgPrintT<T>() << ", Context, InjectionRequirements, " << obf_dbgPrintSeed<seed>() << "," << cycles << ">" << std::endl;
			Context::dbgPrint(offset + 1, "Context:");
			TypeLo::dbgPrint(offset + 1, "TypeLo:");
			RecursiveInjectionLo::dbgPrint(offset + 1, "Lo:");
			TypeHi::dbgPrint(offset + 1, "TypeHi:");
			RecursiveInjectionHi::dbgPrint(offset + 1, "Hi:");
		}
#endif
	};



	#if 0 //COMMENTED OUT - TOO OBVIOUS IN DECOMPILE :-(; if using - rename into "version 8"
	//version 7: 1-bit rotation 
	template<class Context>
	struct obf_injection_version7_descr {
		static constexpr OBFCYCLES own_min_injection_cycles = 5;//relying on compiler generating cmovns etc.
		static constexpr OBFCYCLES own_min_surjection_cycles = 5;//relying on compiler generating cmovns etc.
		static constexpr OBFCYCLES own_min_cycles = Context::context_cycles + Context::calc_cycles(own_min_injection_cycles, own_min_surjection_cycles);
		static constexpr ObfDescriptor descr = ObfDescriptor(true, own_min_cycles, 100);
	};

	template <class T, class Context, ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection_version<7, T, Context, seed, cycles> {
		static_assert(std::is_integral<T>::value);
		static_assert(std::is_unsigned<T>::value);
	public:
		static constexpr OBFCYCLES availCycles = cycles - obf_injection_version7_descr<Context>::own_min_cycles;
		static_assert(availCycles >= 0);

		using RecursiveInjection = obf_injection<T, Context, RecursiveInjectionRequirements,ITHARE_OBF_NEW_PRNG(seed, 1), availCycles + Context::context_cycles>;
		using return_type = typename RecursiveInjection::return_type;
		using ST = typename std::make_signed<T>::type;
		static constexpr T highbit = T(1) << (ObfTraits<T>::nbits - 1);
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injection(T x) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			if ((x % 2) == 0)
				x = x >> 1;
			else
				x = ( x >> 1 ) + highbit;
			return RecursiveInjection::template injection<seedc>(x);
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static T surjection(return_type y) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			T yy = RecursiveInjection::template surjection<seedc>(y);
			ST syy = ST(yy);
			if (syy < 0)
				return yy + yy + 1;
			else
				return yy+yy;
		}

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "obf_injection_version<7/*1-bit rotation*/," << obf_dbgPrintT<T>() << "," << obf_dbgPrintSeed<seed>() << "," << cycles << ">" << std::endl;
			RecursiveInjection::dbgPrint(offset + 1);
		}
#endif
	};
#endif//#if 0

#define ITHARE_OBF_FIRST_USER_INJECTION 8
#include "../obf_user_injection.h"
#ifndef ITHARE_OBF_USER_INJECTION_DESCRIPTOR_LIST
#error "ITHARE_OBF_USER_INJECTION_DESCRIPTOR_LIST MUST be defined in obf_user_injection.h"
#endif

	//obf_injection: choosing one of obf_injection_version<which,...>
	template<class T, class Context, class InjectionRequirements,ITHARE_OBF_SEEDTPARAM seed, OBFCYCLES cycles>
	class obf_injection {
		static_assert(std::is_same<T, typename Context::Type>::value);
		using Traits = ObfTraits<T>;
		constexpr static ObfDescriptor descr[] = {
			obf_injection_version0_descr<Context>::descr,
			obf_injection_version1_descr<Context>::descr,
			obf_injection_version2_descr<T,Context>::descr,
			obf_injection_version3_descr<T,Context>::descr,
			obf_injection_version4_descr<Context>::descr,
			obf_injection_version5_descr<T,Context>::descr,
			obf_injection_version6_descr<T,Context>::descr,
			obf_injection_version7_descr<T,Context,InjectionRequirements>::descr,
			ITHARE_OBF_USER_INJECTION_DESCRIPTOR_LIST//MUST be defined in "../obf_user_injection.h", even if empty
		};
		constexpr static size_t which = obf_random_obf_from_list<ITHARE_OBF_NEW_PRNG(seed, 1)>(cycles, descr,InjectionRequirements::exclude_version);
		static_assert(which >= 0 && which < obf_arraysz(descr));
		using WhichType = obf_injection_version<which, T, Context, InjectionRequirements, seed, cycles>;

	public:
		using return_type = typename WhichType::return_type;
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injection(T x) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			return WhichType::template injection<seedc>(x);
		}
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static T surjection(return_type y) {
			ITHARE_OBF_DECLAREPRNG_INFUNC seedc = ITHARE_OBF_COMBINED_PRNG(seed,seed2);
			return WhichType::template surjection<seedc>(y);
		}

	public:
		static constexpr OBFINJECTIONCAPS injection_caps = WhichType::injection_caps;
		template<ITHARE_OBF_SEEDTPARAM seed2>
		ITHARE_OBF_FORCEINLINE constexpr static return_type injected_add_mod_max_value_ex(return_type base, T x) {
			return WhichType::template injected_add_mod_max_value_ex<seed2>(base,x);
		}

#ifdef ITHARE_OBF_DBG_ENABLE_DBGPRINT
		static void dbgPrint(size_t offset = 0, const char* prefix = "") {
			size_t dbgWhich = obf_random_obf_from_list<ITHARE_OBF_NEW_PRNG(seed, 1)>(cycles, descr,InjectionRequirements::exclude_version);
			assert(dbgWhich==which);
			std::cout << std::string(offset, ' ') << prefix << "obf_injection<"<<obf_dbgPrintT<T>()<<"," << obf_dbgPrintSeed<seed>() << "," << cycles << ">: which=" << which << " dbgWhich=" << dbgWhich << std::endl;
			//std::cout << std::string(offset, ' ') << " Context:" << std::endl;
			Context::dbgPrint(offset + 1,"Context:");
			//std::cout << std::string(offset, ' ') << " Version:" << std::endl;
			WhichType::dbgPrint(offset + 1);
		}
#endif
	};
	
  }//namespace obf
}//namespace ithare
#endif //ITHARE_OBF_SEED

#endif //ithare_obf_injection_h_included
