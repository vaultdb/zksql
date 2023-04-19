#ifndef _AGGREGATE_ID_H
#define _AGGREGATE_ID_H

#include <string>
#include <sstream>

namespace vaultdb {

    enum class AggregateId {
      COUNT,
      SUM,
      AVG,
      MIN,
      MAX
    };


    struct ScalarAggregateDefinition {
        int ordinal; // input ordinal/operand
        AggregateId type;
        std::string alias;
        bool is_distinct = false;

        ScalarAggregateDefinition(int anOrdinal, AggregateId aggregateId, std::string anAlias)
                : ordinal(anOrdinal), type(aggregateId), alias(anAlias) {}

        ScalarAggregateDefinition() {}

        std::string toString() const {
            std::stringstream ss;

            switch(type) {
                case AggregateId::COUNT:
                    ss << "COUNT("
                       << ((ordinal == -1) ? "*" : "$" + std::to_string(ordinal))
                       << ") " << alias;
                    break;
                case AggregateId::SUM:
                    ss << "SUM("
                       << "$" + std::to_string(ordinal)
                       << ") " << alias;
                    break;

                case AggregateId::AVG:
                    ss << "AVG("
                       << "$" + std::to_string(ordinal)
                       << ") " << alias;
                    break;

                case AggregateId::MIN:
                    ss << "SUM("
                       << "$" + std::to_string(ordinal)
                       << ") " << alias;
                    break;

                case AggregateId::MAX:
                    ss << "SUM("
                       << "$" + std::to_string(ordinal)
                       << ") " << alias;
            };

            return ss.str();

            }
        };




}
#endif // _AGGREGATE_ID_H
