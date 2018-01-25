#ifndef ithare_obf_anti_debug_h_included
#define ithare_obf_anti_debug_h_included

//NOT intended to be #included directly
//  #include ../obf.h instead

#if defined(ITHARE_OBF_SEED) && !defined(ITHARE_OBF_NO_ANTI_DEBUG)

#include "obf_common.h"
#include "obf_prng.h"

namespace ithare {
	namespace obf {

/* ************** NAIVE SYSTEM-SPECIFIC **************** */
		
#ifdef _MSC_VER
//TODO: similar stuff for Clang/Win
#include <intrin.h>

	//moving globals into header (along the lines of https://stackoverflow.com/a/27070265)
	template<class Dummy>
	struct ObfNaiveSystemSpecific {
		static volatile uint8_t* obf_peb;
		
		ITHARE_OBF_FORCEINLINE static void init() {//TODO/decide: ?should we obfuscate this function itself?
#ifdef _WIN64
			constexpr auto offset = 0x60;
			obf_peb = (uint8_t*)__readgsqword(offset);
#else
			constexpr auto offset = 0x30;
			obf_peb = (uint8_t*)__readfsdword(offset);
#endif
			return;
		}
		ITHARE_OBF_FORCEINLINE static uint8_t zero_if_not_being_debugged() {
#ifdef ITHARE_OBF_DEBUG_ANTI_DEBUG_ALWAYS_FALSE
			return 0;
#else
			return obf_peb[2];
#endif
		} 
	};

	template<class Dummy>
	volatile uint8_t* ObfNaiveSystemSpecific<Dummy>::obf_peb = nullptr;

//_MSC_VER
#elif defined(__APPLE_CC__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <mach/task.h>
#include <mach/mach_init.h>
	
	//moving globals into header (along the lines of https://stackoverflow.com/a/27070265)
	template<class Dummy>
	struct ObfNaiveSystemSpecific {
		static volatile uint64_t obf_kp_proc_p_flag;
		static volatile mach_port_t obf_mach_port;
		
		ITHARE_OBF_FORCEINLINE static void init() {//TODO/decide: ?should we obfuscate this function itself?
			//#1. Detecting PTRACE: adaptation from https://developer.apple.com/library/content/qa/qa1361/_index.html
			int                 junk;
			int                 mib[4];
			struct kinfo_proc   info;
			size_t              size;

			// Initialize the flags so that, if sysctl fails for some bizarre 
			// reason, we get a predictable result.

			info.kp_proc.p_flag = 0;

			// Initialize mib, which tells sysctl the info we want, in this case
			// we're looking for information about a specific process ID.

			mib[0] = CTL_KERN;
			mib[1] = KERN_PROC;
			mib[2] = KERN_PROC_PID;
			mib[3] = getpid();

			// Call sysctl.

			size = sizeof(info);
			junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
			assert(junk == 0);

			// We're being debugged if the P_TRACED flag is set.

			//return ( (info.kp_proc.p_flag & P_TRACED) != 0 );		
			obf_kp_proc_p_flag = info.kp_proc.p_flag;//we'll check for P_TRACED a bit later
			
			//#2. Detecting Mach: adaptation from https://zgcoder.net/ramblings/osx-debugger-detection.html

			mach_msg_type_number_t count = 0;
			exception_mask_t masks[EXC_TYPES_COUNT];
			mach_port_t ports[EXC_TYPES_COUNT];
			exception_behavior_t behaviors[EXC_TYPES_COUNT];
			thread_state_flavor_t flavors[EXC_TYPES_COUNT];
			exception_mask_t mask = EXC_MASK_ALL & ~(EXC_MASK_RESOURCE | EXC_MASK_GUARD);

			kern_return_t result = task_get_exception_ports(mach_task_self(), mask, masks, &count, ports, behaviors, flavors);
			if (result == KERN_SUCCESS)
			{
				for (mach_msg_type_number_t portIndex = 0; portIndex < count; portIndex++)
				{
					obf_mach_port = ports[portIndex];
					if (MACH_PORT_VALID(obf_mach_port))
						return;//leaving obf_mach_port to a value which will return MACH_PORT_VALID(obf_mach_port) as true
				}
			}
		}
		
		ITHARE_OBF_FORCEINLINE static uint8_t zero_if_not_being_debugged() {
#ifdef ITHARE_OBF_DEBUG_ANTI_DEBUG_ALWAYS_FALSE
			return 0;
#else
			return (obf_kp_proc_p_flag & P_TRACED) + MACH_PORT_VALID(obf_mach_port);
#endif
		}
	};

	template<class Dummy>
	volatile uint64_t ObfNaiveSystemSpecific<Dummy>::obf_kp_proc_p_flag = 0;
	template<class Dummy>
	volatile mach_port_t ObfNaiveSystemSpecific<Dummy>::obf_mach_port = MACH_PORT_NULL;

//__APPLE_CC__
#else
#warning No support for anti-debugging for this platform (yet) - defaulting to simple read-volatile (to make sure that literal usages are still not easily eliminatable)  

	template<class Dummy>
	struct ObfNaiveSystemSpecific {
		static volatile uint32_t zero;
		
		ITHARE_OBF_FORCEINLINE static void init() {//TODO/decide: ?should we obfuscate this function itself?
			zero = 0;
		}
		ITHARE_OBF_FORCEINLINE static uint8_t zero_if_not_being_debugged() {
#ifdef ITHARE_OBF_DEBUG_ANTI_DEBUG_ALWAYS_FALSE
			return 0;
#else
			return zero;
#endif
		} 
	};
	
	template<class Dummy>
	volatile uint32_t ObfNaiveSystemSpecific<Dummy>::zero = 1;//to be owerwritten in init()	
	
#endif // unrecognized platform		

/* ************** TIME-BASED **************** */

	constexpr int obf_bit_upper_bound(uint64_t x) {
	uint64_t test = 1;
	for (int i = 0; i < 64; ++i, test <<= 1) {
		if (test > x)
			return i;
	}
	assert(false);
	return 63;
}

#define ITHARE_OBF_NON_BLOCKING_DAMN_LOT_SECONDS 15

#if defined(_MSC_VER) && ( defined(_M_IX86) || defined(_M_X64))  	

#if defined(_WIN32)//includes _WIN64; I currently prefer reading of SharedUserData on Windows; change to 0 if you still prefer RDTSC
//since Windows 5.2 (late WinXP) TODO: check what happens on pre-XP (guess it should still work but...)
#define ITHARE_OBF_TIME_TYPE uint64_t 
#define ITHARE_OBF_TIME_NOW() (uint64_t((*(uint32_t*)(0x7FFE'0320)))*uint64_t((*(uint32_t*)(0x7FFE'0004)))) //TODO: consider obfuscating 0x7FFExxxx constants
#define ITHARE_OBF_TIME_NON_BLOCKING_THRESHOLD (uint64_t(ITHARE_OBF_NON_BLOCKING_DAMN_LOT_SECONDS*1000)<<0x18)
//TODO: think about polyvariant implementation of ObfNonBlockingCode (some being based on reading SharedUserData, some - on RDTSC)
#else//#if _WIN32
#include <intrin.h>
#define ITHARE_OBF_TIME_TYPE uint64_t
#define ITHARE_OBF_TIME_NOW() __rdtsc() //TODO: consider rewriting manually (MSVC intrinsic tends to exhibit a very obvious pattern, and we don't really need lower word of RDTSC) 
#define ITHARE_OBF_TIME_NON_BLOCKING_THRESHOLD (UINT64_C(4'000'000'000)*ITHARE_OBF_NON_BLOCKING_DAMN_LOT_SECONDS) //4GHz is currently about the absolute-maximum frequency; if frequency is lower - it is even safer
#endif

#elif defined(__clang__) && (defined(__x86_64__)||defined(__i386__))
#include <x86intrin.h>
#define ITHARE_OBF_TIME_TYPE uint64_t
#define ITHARE_OBF_TIME_NOW() __rdtsc()
#define ITHARE_OBF_TIME_NON_BLOCKING_THRESHOLD (UINT64_C(4'000'000'000)*ITHARE_OBF_NON_BLOCKING_DAMN_LOT_SECONDS) //4GHz is currently about the absolute-maximum frequency; if frequency is lower - it is even safer
#else
#warning "Time-based protection is not supported yet for this platform (executable will work, but without time-based anti-debug)"
#define ITHARE_OBF_TIME_TYPE unsigned //we don't really need it
#define ITHARE_OBF_TIME_NOW() 0
#define ITHARE_OBF_TIME_NON_BLOCKING_THRESHOLD 1
#endif


//using template to move static data to header...
template<class Dummy>
struct ObfNonBlockingCodeStaticData {
	static std::atomic<uint32_t> violation_count;
	
	template<ITHARE_OBF_SEEDTPARAM seed2>
	ITHARE_OBF_FORCEINLINE static uint32_t zero_if_not_being_debugged() {
		return violation_count.load();
	} 
};
template<class Dummy>
std::atomic<uint32_t> ObfNonBlockingCodeStaticData<Dummy>::violation_count = 0;


class ObfNonBlockingCode {
	//MUST be used ONLY on-stack
	//MUST NOT be used over potentially-lengthy operations such as ANY over-the-network operations   
	//SHOULD NOT be used to over I/O operations, even when such operations are local (such as disk reads/writes or console writes)
	ITHARE_OBF_TIME_TYPE started;

	public:
	ITHARE_OBF_FORCEINLINE ObfNonBlockingCode() {
		started = ITHARE_OBF_TIME_NOW();
	}
	ITHARE_OBF_FORCEINLINE ~ObfNonBlockingCode() {
		ITHARE_OBF_TIME_TYPE delta = ITHARE_OBF_TIME_NOW() - started;
		constexpr int threshold_bits = obf_bit_upper_bound(ITHARE_OBF_TIME_NON_BLOCKING_THRESHOLD);
		delta >>= threshold_bits;//expected to be zero at this point
		ObfNonBlockingCodeStaticData<void>::violation_count += uint32_t(delta);
	}
	
	//trying to prevent accidental non-stack uses; not bulletproof, but better than nothing
	ObfNonBlockingCode(const ObfNonBlockingCode&) = delete;
	ObfNonBlockingCode& operator =(const ObfNonBlockingCode&) = delete;
	ObfNonBlockingCode(const ObfNonBlockingCode&&) = delete;
	ObfNonBlockingCode& operator =(const ObfNonBlockingCode&&) = delete;
	static void* operator new(size_t) = delete;
	static void* operator new[](size_t) = delete;
};

  }//namespace obf
}//namespace ithare

#undef ITHARE_OBF_TIME_TYPE
#undef ITHARE_OBF_TIME_NOW
#undef ITHARE_OBF_TIME_NON_BLOCKING_THRESHOLD

#else //ITHARE_OBF_SEED && !ITHARE_OBF_NO_ANTI_DEBUG
namespace ithare {
	namespace obf {
		
	template<class Dummy>
	struct ObfNaiveSystemSpecific {
		
		ITHARE_OBF_FORCEINLINE static void init() {
		}
		ITHARE_OBF_FORCEINLINE static uint8_t zero_if_not_being_debugged() {
			return 0;
		} 
	};

class ObfNonBlockingCode {//to be used ONLY on-stack
	public:
	ObfNonBlockingCode() {
	}
	~ObfNonBlockingCode() {
	}
	
	//trying to prevent accidental non-stack uses; not bulletproof, but better than nothing
	ObfNonBlockingCode(const ObfNonBlockingCode&) = delete;
	ObfNonBlockingCode& operator =(const ObfNonBlockingCode&) = delete;
	ObfNonBlockingCode(const ObfNonBlockingCode&&) = delete;
	ObfNonBlockingCode& operator =(const ObfNonBlockingCode&&) = delete;
	static void* operator new(size_t) = delete;
	static void* operator new[](size_t) = delete;
};

  }//namespace obf
}//namespace ithare

#endif //ITHARE_OBF_SEED && !ITHARE_OBF_NO_ANTI_DEBUG

#endif //ithare_obf_anti_debug_h_included
