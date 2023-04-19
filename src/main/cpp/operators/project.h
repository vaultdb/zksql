#ifndef _PROJECT_H
#define _PROJECT_H

#include "operator.h"
#include "common/defs.h"
#include <expression/expression.h>

typedef std::pair<int32_t, int32_t> ProjectionMapping; // src ordinal, dst ordinal

// single projection, it is either an expression over 2+ fields or it is a 1:1 mapping spec'd in projection mapping

typedef std::vector<ProjectionMapping> ProjectionMappingSet;

namespace vaultdb {
    class Project : public Operator {

        std::map<uint32_t, shared_ptr<Expression<bool> > > plain_expressions_; // key = dst_idx, value is expression to evaluate
        std::map<uint32_t, shared_ptr<Expression<emp::Bit> > > secure_expressions_; // key = dst_idx, value is expression to evaluate

        ProjectionMappingSet column_mappings_;

    public:
        Project(Operator *child, std::map<uint32_t, shared_ptr<Expression<bool> > > expressions, const SortDefinition & sort_definition = SortDefinition());
        Project(const ZkQueryTable & src, std::map<uint32_t, shared_ptr<Expression<bool> > > expressions, const SortDefinition & sort_definition = SortDefinition());
        ~Project() = default;


        ZkQueryTable runSelf() override;


    private:
        void project_tuple(PlainTuple &dst_tuple,const PlainTuple &src_tuple) const;
        void project_tuple(SecureTuple &dst_tuple,const SecureTuple &src_tuple) const;

        void setup();

    protected:
        string getOperatorType() const override;

        string getParameters() const override;
    };

    // to create projections with simple 1:1 mappings
    template<typename B>
    class ExpressionMapBuilder {
    public:
        ExpressionMapBuilder(const QuerySchema & input_schema);

        void addMapping(const uint32_t & src_idx, const uint32_t & dst_idx);
        void addExpression(const shared_ptr<Expression<B> > & expression, const uint32_t & dst_idx );

        std::map<uint32_t, shared_ptr<Expression<B> > > getExprs() const { return expressions_; }

    private:
        std::map<uint32_t, shared_ptr<Expression<B> > > expressions_;
        QuerySchema src_schema_;


    };


}
#endif //_PROJECT_H
