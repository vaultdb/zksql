#ifndef _UTILITIES_H
#define _UTILITIES_H

// designed to be minimalist, standalone procedures
// circumvents a dependency loop

// For EMP memory instrumentation
#if defined(__linux__)
#include <sys/time.h>
#include <sys/resource.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/resource.h>
#include <mach/mach.h>
#endif

#include <cstdint>
#include <vector>
#include <string>
#include <emp-tool/circuits/bit.h>
#include <operators/support/aggregate_id.h>
#include <expression/bool_expression.h>
#include <expression/comparator_expression_nodes.h>
#include <expression/generic_expression.h>
#include <common/defs.h>


namespace vaultdb {
    class Utilities {
    public:

        static std::string getCurrentWorkingDirectory();

        static void checkMemoryUtilization(const std::string & msg);

        static size_t checkMemoryUtilization(bool print=false);

        static size_t residentMemoryUtilization(bool print=false);

        static std::string getStackTrace();

        static bool *bytesToBool(int8_t *bytes, int byteCount);

        static std::vector<int8_t> boolsToBytes(const bool *const src, const uint32_t &bitCount);

        static std::vector<int8_t> boolsToBytes( std::string & src); // for the output of Integer::reveal<string>()

        // convert 8 bits to a byte
        static int8_t boolsToByte(const bool *src);

        static std::string revealAndPrintBytes(emp::Bit *bits, const int &byteCount);

        static void mkdir(const std::string & path);

        // for use in plan reader
        static AggregateId getAggregateId(const std::string & src);

        // for use in joins
        // indexes are based on the concatenated tuple, not addressing each input to the join comparison individually
        template<typename B>
       static inline BoolExpression<B> getEqualityPredicate(const uint32_t & lhs_idx, const uint32_t & rhs_idx) {
            std::shared_ptr<InputReferenceNode<B> > lhs_input(new InputReferenceNode<B>(lhs_idx));
            std::shared_ptr<InputReferenceNode<B> > rhs_input(new InputReferenceNode<B>(rhs_idx));
            std::shared_ptr<ExpressionNode<B> > equality_node(new EqualNode<B>(lhs_input, rhs_input));
            return BoolExpression<B>(equality_node);
        }

    };

}

#endif //_UTILITIES_H
