#ifndef _DEFS_H
#define _DEFS_H

#include <emp-tool/utils/constants.h>
#include <vector>
#include <cstdint>
#include <boost/variant.hpp>



namespace vaultdb {


    enum class SortDirection { ASCENDING = 0, DESCENDING = 1, INVALID = 2};

    // ordinal == -1 is for dummy tag, order of columns in vector defines comparison thereof
    typedef std::pair<int32_t, SortDirection> ColumnSort;  // ordinal, direction

    typedef    std::vector<ColumnSort> SortDefinition;


}
#endif // _DEFS_H
