#include "type_inference_visitor.h"
#include <expression/expression_node.h>
#include <expression/math_expression_nodes.h>
#include <expression/comparator_expression_nodes.h>
#include <expression/connector_expression_nodes.h>

using namespace vaultdb;



template<typename B>
TypeInferenceVisitor<B>::TypeInferenceVisitor(std::shared_ptr<ExpressionNode<B>> root, const QuerySchema &input_schema)
        : input_schema_(input_schema), bool_type_(std::is_same_v<bool, B> ? FieldType::BOOL : FieldType::SECURE_BOOL) {
    type_rank_[FieldType::BOOL] = type_rank_[FieldType::SECURE_BOOL] = 0;
    type_rank_[FieldType::INT] = type_rank_[FieldType::SECURE_INT] = 1;
    type_rank_[FieldType::LONG] = type_rank_[FieldType::SECURE_LONG] = 2;
    type_rank_[FieldType::FLOAT] = type_rank_[FieldType::SECURE_FLOAT] = 3;

    root->accept(this);
}

template<typename B>
void TypeInferenceVisitor<B>::visit(InputReferenceNode<B> node) {
    last_expression_type_ = input_schema_.getField(node.read_idx_).getType();
}

template<typename B>
void TypeInferenceVisitor<B>::visit(LiteralNode<B> node) {
    last_expression_type_ = node.payload_.getType();
}


// B has to match up with Field<B> if/when we plug in values to the equation later
template<typename B>
void TypeInferenceVisitor<B>::visit(AndNode<B> node) {
    last_expression_type_ = bool_type_;
}

template<typename B>
void TypeInferenceVisitor<B>::visit(OrNode<B> node) {
    last_expression_type_ = bool_type_;
}

template<typename B>
void TypeInferenceVisitor<B>::visit(NotNode<B> node) {
    last_expression_type_ = bool_type_;
}

template<typename B>
void TypeInferenceVisitor<B>::visit(PlusNode<B> node) {
    resolveBinaryNode(node);
}

template<typename B>
void TypeInferenceVisitor<B>::visit(MinusNode<B> node) {
    resolveBinaryNode(node);
}

template<typename B>
void TypeInferenceVisitor<B>::visit(TimesNode<B> node) {
    resolveBinaryNode(node);

}

template<typename B>
void TypeInferenceVisitor<B>::visit(DivideNode<B> node) {
    resolveBinaryNode(node);
}

template<typename B>
void TypeInferenceVisitor<B>::visit(ModulusNode<B> node) {
    resolveBinaryNode(node);
}

template<typename B>
void TypeInferenceVisitor<B>::visit(EqualNode<B> node) {
    last_expression_type_ = bool_type_;
}

template<typename B>
void TypeInferenceVisitor<B>::visit(NotEqualNode<B> node) {
    last_expression_type_ = bool_type_;
}

template<typename B>
void TypeInferenceVisitor<B>::visit(GreaterThanNode<B> node) {
    last_expression_type_ = bool_type_;
}

template<typename B>
void TypeInferenceVisitor<B>::visit(LessThanNode<B> node) {
    last_expression_type_ = bool_type_;
}

template<typename B>
void TypeInferenceVisitor<B>::visit(GreaterThanEqNode<B> node) {
    last_expression_type_ = bool_type_;
}

template<typename B>
void TypeInferenceVisitor<B>::visit(LessThanEqNode<B> node) {
    last_expression_type_ = bool_type_;
}

template<typename B>
FieldType TypeInferenceVisitor<B>::getExpressionType() const {
    return last_expression_type_;
}


template<typename B>
void TypeInferenceVisitor<B>::resolveBinaryNode(const ExpressionNode<B> &binary_node) {
    binary_node.lhs_->accept(this);
    FieldType lhs_type = last_expression_type_;

    binary_node.rhs_->accept(this);
    FieldType rhs_type = last_expression_type_;

    last_expression_type_ = resolveType(lhs_type, rhs_type);
}

template<typename B>
FieldType TypeInferenceVisitor<B>::resolveType(const FieldType &lhs, const FieldType &rhs) {
    // check for unsupported types
    assert(type_rank_.find(lhs) != type_rank_.end() &&  type_rank_.find(rhs) != type_rank_.end());

    if(type_rank_[lhs] > type_rank_[rhs])
        return lhs;
    return rhs;
}


template<typename B>
void TypeInferenceVisitor<B>::visit(CastNode<B> node) {
    last_expression_type_ =  node.dst_type_;
}

template<typename B>
void TypeInferenceVisitor<B>::visit(CaseNode<B> node) {
    return resolveBinaryNode(node);
}



template class vaultdb::TypeInferenceVisitor<bool>;
template class vaultdb::TypeInferenceVisitor<emp::Bit>;
