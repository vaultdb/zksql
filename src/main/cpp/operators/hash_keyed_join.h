#ifndef _HASH_PKEY_FKEY_JOIN_H
#define _HASH_PKEY_FKEY_JOIN_H

#include "join.h"

#include <util/data_utilities.h>
#include <util/field_utilities.h>

#include <unordered_map>
#include <query_table/primitive_plain_tuple.h>

// primary key foreign key hash join
// produces an output of size |foreign key| relation
namespace vaultdb {
    class HashKeyedJoin : public Join {
    public:
        HashKeyedJoin(Operator *foreignKey, Operator *primaryKey, const BoolExpression<bool> & predicate, const vector<pair<int,int>> & joinKeys, const int & fkey = 0, const SortDefinition & sort = SortDefinition());

        ~HashKeyedJoin() = default;

    protected:
        ZkQueryTable runSelf() override;
        PlainTuple getJoinKeys(QuerySchema &keySchema, PlainTuple &tuple, int offset, bool left);
        string getOperatorType() const override;
        int32_t foreign_key_input_ = 0; // default: lhs = fkey
        vector<pair<int,int>> keys;

    private:
        ZkQueryTable foreignKeyPrimaryKeyHashJoin();
        ZkQueryTable primaryKeyForeignKeyHashJoin();
    };
}
#endif //_HASH_PKEY_FKEY_JOIN_H
