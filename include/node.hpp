#pragma once 

#include <vector>
#include <string>

namespace node {

    enum Node_t
    {
        no_type,
        expr,
        decl,
        cond,
        loop,
        assign,
        scope,
        output
    };

    enum BinOpNode_t
    {
        add,
        sub,
        mul,
        div
    };

    enum BinCompOpNode_t
    {
        equal,
        not_equal,
        greater,
        less,
        greater_or_equal,
        less_or_equal
    };

    enum ExprNode_t
    {
        bin_op,
        bin_comp_op,
        number,
        var,
        input
    };
    
    class Node {
    public:
        Node(Node_t type = no_type) : type_(type) {} 
        virtual ~Node() {}
    private:
        Node_t type_;
    }; // class Node

    class ExprNode : public Node 
    {
    public:
        ExprNode(ExprNode_t type) : Node(Node_t::expr), type_(type) {}
    private:
        ExprNode_t type_;
    }; // class ExprNode

    class BinOpNode final : public ExprNode
    {
    public:
        BinOpNode(BinOpNode_t type, ExprNode *left, ExprNode *right) 
        : ExprNode(ExprNode_t::bin_op), type_(type), left_(left), right_(right) {}
    private:
        BinOpNode_t type_;
        ExprNode *left_, *right_;
    }; // class BinOpNode

    class BinCompOpNode final : public ExprNode
    {
    public:
        BinCompOpNode(BinCompOpNode_t type, ExprNode *left, ExprNode *right) 
        : ExprNode(ExprNode_t::bin_comp_op), type_(type), left_(left), right_(right) {}
    private:
        BinCompOpNode_t type_;
        ExprNode *left_, *right_;
    }; // class BinCompOpNode

    class NumberNode final : public ExprNode
    {
    public:
        NumberNode(int number) : ExprNode(ExprNode_t::number), number_(number) {}
    private:
        int number_;
    }; // class NumberNode

    class InputNode final : public ExprNode
    {
    public:
        InputNode() : ExprNode(ExprNode_t::input) {}
    }; // class InputNode

    class VarNode final : public ExprNode
    {
    public:
        VarNode(const std::string &name) : ExprNode(ExprNode_t::var), name_(std::move(name)) {}
    private:
        std::string name_;
    }; // class VarNode

    class ScopeNode final : public Node
    {
    public:
        ScopeNode() : Node(Node_t::scope), kids_(1) {}
        void AddStatement(Node *child) { kids_.push_back(child); }
    private:
        std::vector<Node*> kids_;
    }; // class ScopeNode

    class DeclNode final : public Node
    {
    public:
        DeclNode(const std::string &name) : Node(Node_t::decl), name_(std::move(name)) {}
    private:
        std::string name_;
    }; // class DeclNode

    class CondNode final : public Node 
    {
    public:
        CondNode(BinCompOpNode *left, ScopeNode *right) : Node(Node_t::cond), left_(left), right_(right) {}
    private:
        BinCompOpNode *left_;
        ScopeNode *right_;
    }; // class CondNode

    class LoopNode final : public Node
    {
    public:
        LoopNode(BinCompOpNode *left, ScopeNode *right) : Node(Node_t::loop), left_(left), right_(right) {}
    private:
        BinCompOpNode *left_;
        ScopeNode *right_;
    }; // class LoopNode 

    class AssignNode final : public Node
    {
    public:
        AssignNode(DeclNode *left, ExprNode *right) : Node(Node_t::assign), left_(left), right_(right) {}
    private:
        DeclNode *left_;
        ExprNode *right_;
    }; // class AssignNode

    class OutputNode final : public Node
    {
    public:
        OutputNode(ExprNode *expr) : Node(Node_t::output), expr_(expr) {}
    private:
        ExprNode *expr_;
    }; // class OutputNode

}; // namespace node
