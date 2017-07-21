#include "pe_factory.h"
#include "pe_properties_generic.h"

namespace pe_bliss
{
pe_base pe_factory::create_pe(std::istream& file, bool read_debug_raw_data)
{
	return pe_base::get_pe_type(file) == pe_type_32
		? pe_base(file, pe_properties_32(), read_debug_raw_data)
		: pe_base(file, pe_properties_64(), read_debug_raw_data);
}
}
