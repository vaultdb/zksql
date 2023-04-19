#ifndef _EXPRESSION_VISITOR_H
#define _EXPRESSION_VISITOR_H




namespace vaultdb {

    template<typename B> class InputReferenceNode;
    template<typename B> class  LiteralNode;
    
    template<typename B> class  AndNode;
    template<typename B> class  OrNode;
    template<typename B> class  NotNode;

    template<typename B> class  PlusNode;
    template<typename B> class  MinusNode;
    template<typename B> class  TimesNode;
    template<typename B> class  DivideNode;
    template<typename B> class  ModulusNode;

    template<typename B> class  EqualNode;
    template<typename B> class  NotEqualNode;
    template<typename B> class  GreaterThanNode;
    template<typename B> class  LessThanNode;
    template<typename B> class  GreaterThanEqNode;
    template<typename B> class  LessThanEqNode;
    template<typename B> class  CastNode;

    template<typename B> class  CaseNode;

    template<typename B> class  ExpressionNode;




    // any visitor state is stored in implementing visitor
    template<typename B>
    class ExpressionVisitor {
    public:
        // InputReferenceNode.accept will call this
       virtual void visit(InputReferenceNode<B> node)  = 0;
       virtual void visit(LiteralNode<B> node)  = 0;

        virtual void visit(AndNode<B> node) = 0;
        virtual void visit(OrNode<B> node) = 0;
        virtual void visit(NotNode<B> node) = 0;

        virtual void visit(PlusNode<B> node) = 0;
        virtual void visit(MinusNode<B> node) = 0;
        virtual void visit(TimesNode<B> node) = 0;
        virtual void visit(DivideNode<B> node) = 0;
        virtual void visit(ModulusNode<B> node) = 0;

        virtual void visit(EqualNode<B> node) = 0;
        virtual void visit(NotEqualNode<B> node) = 0;
        virtual void visit(GreaterThanNode<B> node) = 0;
        virtual void visit(LessThanNode<B> node) = 0;
        virtual void visit(GreaterThanEqNode<B> node) = 0;
        virtual void visit(LessThanEqNode<B> node) = 0;
        virtual void visit(CastNode<B> node) = 0;
        virtual void visit(CaseNode<B> node) = 0;

    };

}
#endif //_EXPRESSION_VISITOR_H
