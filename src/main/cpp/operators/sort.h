#include <memory>
#include "common/defs.h"
#include "operator.h"
#include <query_table/query_table.h>
#include <vector>
#include <query_table/primitive_plain_tuple.h>
#include <util/field_utilities.h>


// based on EMP Sort
namespace  vaultdb {
    class Sort : public Operator {


    public:
        Sort(Operator *child, const SortDefinition &aSortDefinition, const int & limit = -1);
        Sort(const ZkQueryTable & child, const SortDefinition &aSortDefinition, const int & limit = -1);
        ~Sort();

        ZkQueryTable runSelf() override;




        static bool lt(const PrimitivePlainTuple & lhs, const PrimitivePlainTuple & rhs, const SortDefinition  & sort_definition);

        template<typename B>
        static B checkOrder(const QueryTuple<B> & lhs, const QueryTuple<B> & rhs, const SortDefinition  & sort_definition) {
            B correctOrder = true;
            B allEq = true;
            //B sortByDummy = false;

            for (size_t i = 0; i < sort_definition.size(); ++i) {

                const Field<B> lhsField = sort_definition[i].first == -1 ? lhs.getDummyTag() : lhs.getField(sort_definition[i].first);
                const Field<B> rhsField = sort_definition[i].first == -1 ? rhs.getDummyTag() : rhs.getField(sort_definition[i].first);

                bool asc = (sort_definition[i].second == SortDirection::ASCENDING);

                B eq = lhsField == rhsField;
                B lt = lhsField < rhsField;
                B colSortMatch = (lt == asc);

                // find first one where not eq, use this to allEq flag
                correctOrder = FieldUtilities::select(allEq, (eq & correctOrder) | ((!eq) & colSortMatch), correctOrder);
                allEq = allEq & eq;
            }

            // ignore dummy tuples
            B dummy = lhs.getDummyTag() | rhs.getDummyTag();

            return dummy | correctOrder;

        }


    protected:
        string getOperatorType() const override;

        string getParameters() const override;

    private:

        int limit_; // -1 means no limit op


    };

    // simple wrapper to integreate sort_definition
    struct tuple_less {
        // returns true if lhs < rhs
        bool operator()(const PrimitivePlainTuple & lhs, const PrimitivePlainTuple & rhs) const {
            return Sort::lt(lhs, rhs, sort_definition);
        }

        tuple_less(SortDefinition a_sort) {
            sort_definition = a_sort;
        }
        SortDefinition sort_definition;
    };



}

