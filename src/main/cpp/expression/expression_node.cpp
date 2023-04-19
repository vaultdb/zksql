#include "expression_node.h"
#include <expression/visitor/print_expression_visitor.h>
#include <query_table/plain_tuple.h>
#include <query_table/secure_tuple.h>
#include <field/field_factory.h>


using namespace vaultdb;

template<typename B>
ExpressionNode<B>::ExpressionNode(std::shared_ptr<ExpressionNode<B>> lhs) : lhs_(lhs) {}

template<typename B>
ExpressionNode<B>::ExpressionNode(std::shared_ptr<ExpressionNode<B>> lhs, std::shared_ptr<ExpressionNode<B>> rhs) : lhs_(lhs), rhs_(rhs) {}

template<typename B>
std::string ExpressionNode<B>::toString()  {
    PrintExpressionVisitor<B> visitor;
    this->accept(&visitor);
    return visitor.getString();

}


std::ostream &vaultdb::operator<<(std::ostream &os,  ExpressionNode<bool> &expression) {
    PrintExpressionVisitor<bool> visitor;
    expression.accept(&visitor);

    os << visitor.getString();

    return os;
}

std::ostream &vaultdb::operator<<(std::ostream &os,  ExpressionNode<emp::Bit> &expression) {
    PrintExpressionVisitor<emp::Bit> visitor;
    expression.accept(&visitor);

    os << visitor.getString();

    return os;

}

template<typename B>
InputReferenceNode<B>::InputReferenceNode(const uint32_t &read_idx) : ExpressionNode<B>(nullptr), read_idx_(read_idx) {}

template<typename B>
Field<B> InputReferenceNode<B>::call(const QueryTuple<B> &target) const {
    return target.getField(read_idx_);
}

template<typename B>
ExpressionKind InputReferenceNode<B>::kind() const {
    return ExpressionKind::INPUT_REF;
}

template<typename B>
void InputReferenceNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}



template<typename B>
LiteralNode<B>::LiteralNode(const Field<B> &literal) : ExpressionNode<B>(nullptr), payload_(literal) {

}

template<typename B>
Field<B> LiteralNode<B>::call(const QueryTuple<B> &target) const {
    return payload_;
}

template<typename B>
ExpressionKind LiteralNode<B>::kind() const {
    return ExpressionKind::LITERAL;
}

template<typename B>
void vaultdb::LiteralNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}

template<typename B>
std::shared_ptr <LiteralNode<emp::Bit> > LiteralNode<B>::toSecure() const {
    return std::shared_ptr<LiteralNode<emp::Bit> >(new LiteralNode<emp::Bit>(payload_.secret_share()));
}

template<typename B>
CastNode<B>::CastNode( std::shared_ptr<ExpressionNode<B> > & input, const FieldType &dst_type) : ExpressionNode<B>(input),  dst_type_(dst_type) { }

template<typename B>
Field<B> CastNode<B>::call(const QueryTuple<B> &target) const {
    // attempt to cast
    Field<B> src =  ExpressionNode<B>::lhs_->call(target);

    switch(dst_type_) {
        case FieldType::INT:
        case FieldType::SECURE_INT:
            return FieldFactory<B>::toInt(src);
        case FieldType::LONG:
        case FieldType::SECURE_LONG:
            return FieldFactory<B>::toLong(src);
        case FieldType::FLOAT:
        case FieldType::SECURE_FLOAT:
            return FieldFactory<B>::toFloat(src);
        default:
            throw;
    }

}

template<typename B>
ExpressionKind CastNode<B>::kind() const {
    return ExpressionKind::CAST;

}

template<typename B>
void CastNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}




template class vaultdb::ExpressionNode<bool>;
template class vaultdb::ExpressionNode<emp::Bit>;

template class vaultdb::InputReferenceNode<bool>;
template class vaultdb::InputReferenceNode<emp::Bit>;

template class vaultdb::LiteralNode<bool>;
template class vaultdb::LiteralNode<emp::Bit>;

template class vaultdb::CastNode<bool>;
template class vaultdb::CastNode<emp::Bit>;

