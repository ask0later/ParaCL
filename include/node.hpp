#pragma once 
#include <vector>
#include <string>
#include <memory>
#include "location.hpp"

namespace node {
    namespace details {
        template <typename T> class Builder final {
        public:
            template <typename S, typename... Args>
            S *GetObj(Args&&... args) {
                auto tmp = std::make_unique<S>(std::forward<Args>(args)...);
                auto raw_ptr = tmp.get();
                buffer_.push_back(std::move(tmp));
                return raw_ptr;
            }
        private:
            std::vector<std::unique_ptr<T>> buffer_;
        }; // class Builder
    } // namespace details

    enum Node_t {
        no_type = 1,
        expr,
        decl,
        cond,
        loop, 
        scope,
        output
    };

    enum UnOpNode_t {
        minus = 10,
        negation
    };

    enum BinOpNode_t {
        add = 20,
        sub,
        mul,
        div,
        remainder
    };

    enum BinCompOpNode_t {
        equal = 30,
        not_equal,
        greater,
        less,
        greater_or_equal,
        less_or_equal
    };

    enum LogicOpNode_t {
        logic_and = 40,
        logic_or
    };

    enum ExprNode_t {
        assign = 50,
        logic_op,
        un_op,
        bin_op,
        bin_comp_op,
        number,
        var,
        input
    };

    class NodeVisitor;

    struct Node {
    public:
        Node(Node_t type, yy::Location location) :
            type_(type), location_(location) {}
        virtual ~Node() = default;
        inline virtual void Accept(NodeVisitor &visitor) = 0;
        Node_t type_;
        yy::Location location_;
    }; // class Node

    struct ExprNode : public Node {
        ExprNode(ExprNode_t type, yy::Location location) : Node(Node_t::expr, location), type_(type) {}
        inline void Accept(NodeVisitor &visitor) override = 0;
        ExprNode_t type_;
    }; // class ExprNode

    struct ScopeNode final : public Node {
        ScopeNode(yy::Location location) : Node(Node_t::scope, location), kids_(1) {}
        inline void Accept(NodeVisitor &visitor) override;
        void AddStatement(Node *child) { kids_.push_back(child); }
        std::vector<Node*> kids_;
    }; // class ScopeNode

    struct DeclNode final : public Node {
        DeclNode(const std::string &name, yy::Location location) : Node(Node_t::decl, location), name_(std::move(name)) {}
        inline void Accept(NodeVisitor &visitor) override;
        std::string name_;
    }; // class DeclNode

    struct CondNode final : public Node {
        CondNode(ExprNode *predicat, ScopeNode *first, ScopeNode *second, yy::Location location) :
            Node(Node_t::cond, location), predicat_(predicat), first_(first), second_(second) {}
        inline void Accept(NodeVisitor &visitor) override;
        ExprNode *predicat_;
        ScopeNode *first_;
        ScopeNode *second_;
    }; // class CondNode

    struct LoopNode final : public Node {
        LoopNode(ExprNode *predicat, ScopeNode *scope, yy::Location location) : Node(Node_t::loop, location), predicat_(predicat), scope_(scope) {}
        inline void Accept(NodeVisitor &visitor) override;
        ExprNode *predicat_;
        ScopeNode *scope_;
    }; // class LoopNode 

    struct OutputNode final : public Node {
        OutputNode(ExprNode *expr, yy::Location location) : Node(Node_t::output, location), expr_(expr) {}
        inline void Accept(NodeVisitor &visitor) override;
        ExprNode *expr_;
    }; // class OutputNode

    struct LogicOpNode final : public ExprNode {
        LogicOpNode(LogicOpNode_t type, ExprNode *left, ExprNode *right, yy::Location location) 
        : ExprNode(ExprNode_t::logic_op, location), type_(type), left_(left), right_(right) {}
        inline void Accept(NodeVisitor &visitor) override;
        LogicOpNode_t type_;
        ExprNode *left_;
        ExprNode *right_;
    }; // class LogicNode

    struct UnOpNode final : public ExprNode {
        UnOpNode(UnOpNode_t type, ExprNode *child, yy::Location location) 
        : ExprNode(ExprNode_t::un_op, location), type_(type), child_(child) {}
        inline void Accept(NodeVisitor &visitor) override;
        UnOpNode_t type_;
        ExprNode *child_;
    }; // class UnOpNode

    struct BinOpNode final : public ExprNode {
        BinOpNode(BinOpNode_t type, ExprNode *left, ExprNode *right, yy::Location location) 
        : ExprNode(ExprNode_t::bin_op, location), type_(type), left_(left), right_(right) {}
        inline void Accept(NodeVisitor &visitor) override;
        BinOpNode_t type_;
        ExprNode *left_, *right_;
    }; // class BinOpNode

    struct BinCompOpNode final : public ExprNode {
        BinCompOpNode(BinCompOpNode_t type, ExprNode *left, ExprNode *right, yy::Location location) 
        : ExprNode(ExprNode_t::bin_comp_op, location), type_(type), left_(left), right_(right) {}
        inline void Accept(NodeVisitor &visitor) override;
        BinCompOpNode_t type_;
        ExprNode *left_, *right_;
    }; // class BinCompOpNode

    struct NumberNode final : public ExprNode {
        NumberNode(int number, yy::Location location) : ExprNode(ExprNode_t::number, location), number_(number) {}
        inline void Accept(NodeVisitor &visitor) override;
        int number_;
    }; // class NumberNode

    struct InputNode final : public ExprNode {
        InputNode(yy::Location location) : ExprNode(ExprNode_t::input, location) {}
        inline void Accept(NodeVisitor &visitor) override;
    }; // class InputNode

    struct VarNode final : public ExprNode {
        VarNode(const std::string &name, yy::Location location) : ExprNode(ExprNode_t::var, location), name_(std::move(name)) {}
        inline void Accept(NodeVisitor &visitor) override;
        std::string name_;
    }; // class VarNode

    struct AssignNode final : public ExprNode {
        AssignNode(DeclNode *var, ExprNode *expr, yy::Location location) : ExprNode(ExprNode_t::assign, location), var_(var), expr_(expr) {}
        inline void Accept(NodeVisitor &visitor) override;
        DeclNode *var_;
        ExprNode *expr_;
    }; // class AssignNode

    class NodeVisitor {
    public:
        virtual ~NodeVisitor() = default;
        virtual void Visit(LogicOpNode &node) = 0;
        virtual void Visit(UnOpNode &node) = 0;
        virtual void Visit(BinOpNode &node) = 0;
        virtual void Visit(BinCompOpNode &node) = 0;
        virtual void Visit(NumberNode &node) = 0;
        virtual void Visit(InputNode &node) = 0;
        virtual void Visit(VarNode &node) = 0;
        virtual void Visit(ScopeNode &node) = 0;
        virtual void Visit(DeclNode &node) = 0;
        virtual void Visit(CondNode &node) = 0;
        virtual void Visit(LoopNode &node) = 0;
        virtual void Visit(AssignNode &node) = 0;
        virtual void Visit(OutputNode &node) = 0;
    }; // class NodeVisitor

    inline void LogicOpNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void UnOpNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void BinOpNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void BinCompOpNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void NumberNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void InputNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void VarNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void ScopeNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void DeclNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void CondNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void LoopNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void AssignNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }

    inline void OutputNode::Accept(NodeVisitor &visitor) {
        visitor.Visit(*this);
    }
}; // namespace node