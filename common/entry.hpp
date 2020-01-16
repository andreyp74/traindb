#ifndef ENTRY_HPP_
#define ENTRY_HPP_

#include "storage_types.hpp"

struct Entry
{
    key_type key;
	value_type value;
	ver_type version;
};


#endif //ENTRY_HPP_