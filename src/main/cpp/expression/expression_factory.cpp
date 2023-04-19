#include "expression_factory.h"

using namespace vaultdb;

template<typename B>
std::shared_ptr<ExpressionNode<B>>
ExpressionFactory<B>::getExpressionNode(const string &expression_kind, std::shared_ptr<ExpressionNode<B>> lhs,
std::shared_ptr<ExpressionNode<B>> rhs) {
    ExpressionKind kind = getKind(expression_kind);
    return getExpressionNode(kind, lhs, rhs);
}


template<typename B>
std::shared_ptr<ExpressionNode<B>>
ExpressionFactory<B>::getExpressionNode(const string &expression_kind, vector<std::shared_ptr<ExpressionNode<B> > >  & operands) {
    ExpressionKind kind = getKind(expression_kind);

    if(kind == ExpressionKind::CASE) {
        assert(operands.size() == 3);
       BoolExpression<B> conditional(operands[0]);
        return std::shared_ptr<ExpressionNode<B> >(new CaseNode<B>(conditional, operands[1], operands[2]));
    }
    else
        return getExpressionNode(kind, operands[0], operands[1]);
}

template<typename B>
std::shared_ptr<ExpressionNode<B>>
ExpressionFactory<B>::getExpressionNode(const ExpressionKind &kind, std::shared_ptr<ExpressionNode<B>> lhs,
std::shared_ptr<ExpressionNode<B>> rhs) {
    switch(kind) {
        case ExpressionKind::PLUS:
            return std::shared_ptr<ExpressionNode<B> >(new PlusNode<B>(lhs, rhs));
        case ExpressionKind::MINUS:
            return std::shared_ptr<ExpressionNode<B> >(new MinusNode<B>(lhs, rhs));
        case ExpressionKind::TIMES:
            return std::shared_ptr<ExpressionNode<B> >(new TimesNode<B>(lhs, rhs));
        case ExpressionKind::DIVIDE:
            return std::shared_ptr<ExpressionNode<B> >(new DivideNode<B>(lhs, rhs));
        case ExpressionKind::MOD:
            return std::shared_ptr<ExpressionNode<B> >(new ModulusNode<B>(lhs, rhs));
        case ExpressionKind::AND:
            return std::shared_ptr<ExpressionNode<B> >(new AndNode<B>(lhs, rhs));
        case ExpressionKind::OR:
            return std::shared_ptr<ExpressionNode<B> >(new OrNode<B>(lhs, rhs));
        case ExpressionKind::NOT:
            return std::shared_ptr<ExpressionNode<B> >(new NotNode<B>(lhs));
        case ExpressionKind::EQ:
            return std::shared_ptr<ExpressionNode<B> >(new EqualNode<B>(lhs, rhs));
        case ExpressionKind::NEQ:
            return std::shared_ptr<ExpressionNode<B> >(new NotEqualNode<B>(lhs, rhs));
        case ExpressionKind::LT:
            return std::shared_ptr<ExpressionNode<B> >(new LessThanNode<B>(lhs, rhs));
        case ExpressionKind::LEQ:
            return std::shared_ptr<ExpressionNode<B> >(new LessThanEqNode<B>(lhs, rhs));
        case ExpressionKind::GT:
            return std::shared_ptr<ExpressionNode<B> >(new GreaterThanNode<B>(lhs, rhs));
        case ExpressionKind::GEQ:
            return std::shared_ptr<ExpressionNode<B> >(new GreaterThanEqNode<B>(lhs, rhs));
        default:
            throw new std::invalid_argument("Can't create ExpressionNode for kind " + std::to_string((int) kind));
    }
}

template<typename B>
ExpressionKind ExpressionFactory<B>::getKind(const string &expression_kind) {
    if(expression_kind == "LITERAL") return ExpressionKind::LITERAL;
    if(expression_kind == "INPUT") return ExpressionKind::INPUT_REF;


    if(expression_kind == "PLUS") return ExpressionKind::PLUS;
    if(expression_kind == "MINUS") return ExpressionKind::MINUS;
    if(expression_kind == "TIMES") return ExpressionKind::TIMES;
    if(expression_kind == "DIVIDE") return ExpressionKind::DIVIDE;
    if(expression_kind == "MODULUS") return ExpressionKind::MOD;

    if(expression_kind == "AND") return ExpressionKind::AND;
    if(expression_kind == "OR") return ExpressionKind::OR;
    if(expression_kind == "NOT") return ExpressionKind::NOT;

    if(expression_kind == "EQUALS") return ExpressionKind::EQ;
    if(expression_kind == "NOT_EQUALS") return ExpressionKind::NEQ;
    if(expression_kind == "GREATER_THAN") return ExpressionKind::GT;
    if(expression_kind == "GREATER_THAN_OR_EQUAL") return ExpressionKind::GEQ;
    if(expression_kind == "LESS_THAN") return ExpressionKind::LT;
    if(expression_kind == "LESS_THAN_OR_EQUAL") return ExpressionKind::LEQ;
    if(expression_kind == "CASE") return ExpressionKind::CASE;
    throw new std::invalid_argument("Can't create ExpressionKind for  " + expression_kind);


}

template class vaultdb::ExpressionFactory<bool>;
template class vaultdb::ExpressionFactory<emp::Bit>;
