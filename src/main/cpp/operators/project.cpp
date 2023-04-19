#include "project.h"
#include <query_table/field/field_factory.h>
#include <query_table/secure_tuple.h>
#include <expression/expression_node.h>
#include <expression/generic_expression.h>
#include <util/data_utilities.h>
#include <expression/visitor/plain_to_secure_visitor.h>

using namespace vaultdb;




Project::Project(Operator *child, std::map<uint32_t, shared_ptr<Expression<bool> > > expression_map, const SortDefinition & sort_definition) :
    Operator(child, sort_definition), plain_expressions_(expression_map) {

    for(auto expr_pos =  plain_expressions_.begin(); expr_pos != plain_expressions_.end(); ++expr_pos)  {
        GenericExpression<bool> *plain_expr = (GenericExpression<bool> *) expr_pos->second.get();

        PlainToSecureVisitor visitor(plain_expr->root_);
        shared_ptr<Expression<emp::Bit> > secure_expr(new GenericExpression<emp::Bit>(visitor.getSecureExpression(),
                                                                                      child->getOutputSchema()));
        uint32_t dst_column = expr_pos->first;
        secure_expressions_[dst_column] = secure_expr;
    }

    setup();
}


Project::Project(const ZkQueryTable & child, std::map<uint32_t, shared_ptr<Expression<bool> > > expression_map, const SortDefinition & sort) : Operator(child, sort), plain_expressions_(expression_map) {
    for(auto expr_pos =  plain_expressions_.begin(); expr_pos != plain_expressions_.end(); ++expr_pos)  {
        GenericExpression<bool> *plain_expr = (GenericExpression<bool> *) expr_pos->second.get();

        PlainToSecureVisitor visitor(plain_expr->root_);
        shared_ptr<Expression<emp::Bit> > secure_expr(new GenericExpression<emp::Bit>(visitor.getSecureExpression(),
                                                                                      *child.getSchema()));
        uint32_t dst_column = expr_pos->first;
        secure_expressions_[dst_column] = secure_expr;
    }

    setup();

}




ZkQueryTable Project::runSelf() {
    ZkQueryTable src_table = getChild(0)->getOutput();
    uint64_t commCostFromAllThreadsBefore = gv.getAccumulativeCommCostFromAllThreads();

    startTime = clock_start();

    uint32_t tuple_cnt_ = src_table.getTupleCount();


    int party = src_table.party_;
    output_ = ZkQueryTable(tuple_cnt_, output_schema_, party, src_table.netio_, sort_definition_);

    for(uint32_t i = 0; i < tuple_cnt_; ++i) {
        if(party == emp::ALICE) { // project plaintext too
            PlainTuple  src_tuple = src_table.plain_table_->getTuple(i);
            PlainTuple dst_tuple = output_.plain_table_->getTuple(i);
            project_tuple(dst_tuple, src_tuple);
        }

        SecureTuple src_tuple = src_table.secure_table_->getTuple(i);
        SecureTuple dst_tuple = output_.secure_table_->getTuple(i);
        project_tuple(dst_tuple, src_tuple);
    }

    uint64_t commCostFromAllThreadsAfter = gv.getAccumulativeCommCostFromAllThreads();
    comm_cost = commCostFromAllThreadsAfter - commCostFromAllThreadsBefore;
    gv.setCommCost(gv.getCommCost() + comm_cost);

    return output_;
}



void Project::project_tuple(PlainTuple &dst_tuple, const PlainTuple  &src_tuple) const {
    dst_tuple.setDummyTag(src_tuple.getDummyTag());

   auto exprPos = plain_expressions_.begin();


    // exec all expressions
    while(exprPos != plain_expressions_.end()) {
        uint32_t dst_ordinal = exprPos->first;
        PlainExpression *expression = exprPos->second.get();
        PlainField field_value = expression->call(src_tuple);
        dst_tuple.setField(dst_ordinal, field_value);
        ++exprPos;
    }


}

void Project::project_tuple(SecureTuple &dst_tuple, const SecureTuple &src_tuple) const {
    dst_tuple.setDummyTag(src_tuple.getDummyTag());

    auto exprPos = secure_expressions_.begin();


    // exec all expressions
    while(exprPos != secure_expressions_.end()) {
        uint32_t dst_ordinal = exprPos->first;
        SecureExpression *expression = exprPos->second.get();
        SecureField field_value = expression->call(src_tuple);
        dst_tuple.setField(dst_ordinal, field_value);
        ++exprPos;
    }


}


void Project::setup() {

    Operator *child = getChild();
    SortDefinition src_sort_order = child->getSortOrder();
    QuerySchema src_schema = child->getOutputSchema();

    assert(plain_expressions_.size() > 0);

    uint32_t col_count = plain_expressions_.size();

    QuerySchema dst_schema = QuerySchema(col_count); // re-initialize it

    for(auto expr_pos =  plain_expressions_.begin(); expr_pos != plain_expressions_.end(); ++expr_pos) {
        if(expr_pos->second->kind() == ExpressionKind::INPUT_REF) {
            GenericExpression<bool> *expr = (GenericExpression<bool> *) expr_pos->second.get();
            InputReferenceNode<bool> *node = (InputReferenceNode<bool> *) expr->root_.get();
            column_mappings_.template emplace_back(node->read_idx_, expr_pos->first);
        }
    }

        // propagate names and specs in column mappings
    for(ProjectionMapping mapping_idx : column_mappings_) {
        uint32_t src_ordinal = mapping_idx.first;
        uint32_t dst_ordinal = mapping_idx.second;
        QueryFieldDesc src_field_desc = src_schema.getField(src_ordinal);
        QueryFieldDesc dst_field_desc(src_field_desc, dst_ordinal);
        dst_schema.putField(dst_field_desc);

        assert(plain_expressions_.find(dst_ordinal) != plain_expressions_.end());
        std::shared_ptr<PlainExpression > expression = plain_expressions_[dst_ordinal];
        expression->setType(src_field_desc.getType());
        expression->setAlias(src_field_desc.getName());

    }


    // set up dst schema for new expressions
    for(auto expr_pos =  plain_expressions_.begin(); expr_pos != plain_expressions_.end(); ++expr_pos) {
        uint32_t dst_ordinal = expr_pos->first;
        QueryFieldDesc dst_field_desc = dst_schema.getField(dst_ordinal);

        if(dst_field_desc.getType() == FieldType::INVALID) { //  not initialized yet
            FieldType dst_type = expr_pos->second->getType();
            std::string dst_name = expr_pos->second->getAlias();

            dst_field_desc = QueryFieldDesc(dst_ordinal, dst_name, "", dst_type, 0); // 0 because string expressions are not yet implemented
            dst_schema.putField(dst_field_desc);
        }
    }


    // confirm that all ordinals are defined
    for(uint32_t i = 0; i < col_count; ++i) {
        assert(plain_expressions_.find(i) != plain_expressions_.end());
    }


    SortDefinition  dst_sort;

    // *** Check to see if order-by carries over
    for(ColumnSort sort : src_sort_order) {
        int32_t src_ordinal = sort.first;
        bool found = false;
        if(src_ordinal == -1) {
            found = true; // dummy tag automatically carries over
            dst_sort.push_back(sort);
        }
        for(ProjectionMapping mapping : column_mappings_) {
            if(mapping.first == src_ordinal) {
                dst_sort.push_back(ColumnSort (mapping.second, sort.second));
                found = true;
                break;
            }
        } // end search for mapping
        if(!found) {
            break;
        } // broke the sequence of mappings
    }

    output_schema_ = QuerySchema::toSecure(dst_schema);
    setSortOrder(dst_sort);

}


string Project::getOperatorType() const {
    return "Project";
}


string Project::getParameters() const {
    stringstream ss;

    auto expr_pos = plain_expressions_.begin();
    ss << "(" << "<" << expr_pos->first << ", " << expr_pos->second->toString() << ">";
    ++expr_pos;
    while(expr_pos != plain_expressions_.end()) {
        ss << ", "  << "<" << expr_pos->first << ", " << expr_pos->second->toString() << ">";
        ++expr_pos;
    }

    ss << ")";

    return ss.str();
}

// **** ExpressionMapBuilder **** //
template<typename B>
ExpressionMapBuilder<B>::ExpressionMapBuilder(const QuerySchema &input_schema) : src_schema_(input_schema) {}


template<typename B>
void ExpressionMapBuilder<B>::addMapping(const uint32_t &src_idx, const uint32_t &dst_idx) {
    shared_ptr<ExpressionNode<B> > node(new InputReferenceNode<B>(src_idx));
    shared_ptr<GenericExpression<B> > expr(new GenericExpression<B>(node, src_schema_));

    expressions_[dst_idx] = expr;
}

template<typename B>
void ExpressionMapBuilder<B>::addExpression(const shared_ptr<Expression<B>> &expression, const uint32_t &dst_idx) {
    expressions_[dst_idx] = expression;
}





template class vaultdb::ExpressionMapBuilder<bool>;
template class vaultdb::ExpressionMapBuilder<emp::Bit>;
