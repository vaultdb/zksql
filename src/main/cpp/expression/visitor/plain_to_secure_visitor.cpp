#include "plain_to_secure_visitor.h"

using namespace vaultdb;
PlainToSecureVisitor::PlainToSecureVisitor(std::shared_ptr<ExpressionNode<bool> > root) {
    root->accept(this);

}

void PlainToSecureVisitor::visit(InputReferenceNode<bool> node) {
    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new InputReferenceNode<emp::Bit>(node.read_idx_));
}

void PlainToSecureVisitor::visit(LiteralNode<bool> node) {
    root_ =   node.toSecure();
}

void PlainToSecureVisitor::visit(AndNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new AndNode<emp::Bit>(lhs, rhs));


}

void PlainToSecureVisitor::visit(OrNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new OrNode<emp::Bit>(lhs, rhs));

}

void PlainToSecureVisitor::visit(NotNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > in = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new NotNode<emp::Bit>(in));


}

void PlainToSecureVisitor::visit(PlusNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new PlusNode<emp::Bit>(lhs, rhs));

}

void PlainToSecureVisitor::visit(MinusNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new MinusNode<emp::Bit>(lhs, rhs));


}

void PlainToSecureVisitor::visit(TimesNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new TimesNode<emp::Bit>(lhs, rhs));

}

void PlainToSecureVisitor::visit(DivideNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new DivideNode<emp::Bit>(lhs, rhs));


}

void PlainToSecureVisitor::visit(ModulusNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new ModulusNode<emp::Bit>(lhs, rhs));


}

void PlainToSecureVisitor::visit(EqualNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new EqualNode<emp::Bit>(lhs, rhs));


}

void PlainToSecureVisitor::visit(NotEqualNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new NotEqualNode<emp::Bit>(lhs, rhs));


}

void PlainToSecureVisitor::visit(GreaterThanNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new GreaterThanNode<emp::Bit>(lhs, rhs));


}

void PlainToSecureVisitor::visit(LessThanNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new LessThanNode<emp::Bit>(lhs, rhs));


}

void PlainToSecureVisitor::visit(GreaterThanEqNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new GreaterThanEqNode<emp::Bit>(lhs, rhs));


}

void PlainToSecureVisitor::visit(LessThanEqNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new LessThanEqNode<emp::Bit>(lhs, rhs));

}

void PlainToSecureVisitor::visit(CastNode<bool> node) {
    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > input = root_;
    FieldType dst_type = TypeUtilities::toSecure(node.dst_type_);

    root_ =  std::shared_ptr<ExpressionNode<emp::Bit> >(new CastNode<emp::Bit>(input, dst_type));
}

void PlainToSecureVisitor::visit(CaseNode<bool> node) {
    node.conditional_.root_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > conditional_node = root_;
    BoolExpression<emp::Bit>  conditional(conditional_node);

    node.lhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > lhs = root_;

    node.rhs_->accept(this);
    std::shared_ptr<ExpressionNode<emp::Bit> > rhs = root_;

    root_ = std::shared_ptr<ExpressionNode<emp::Bit> >(new CaseNode<emp::Bit>(conditional, lhs, rhs));


}


