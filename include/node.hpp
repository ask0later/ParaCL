#pragma once 
#include <vector>
#include <string>
#include "location.hpp"

namespace node {
    namespace details {
        template <typename T> class Builder final {
        public:
            template <typename S, typename... Args>
            S *GetObj(Args... args) {
                auto tmp = new S{args...};
                buffer_.push_back(static_cast<T *>(tmp));
                return tmp;
            }

            void Clean() noexcept {
                for (auto &&tmp : buffer_)
                    delete tmp;
                buffer_.clear();
            }

        private:
            std::vector<T *> buffer_;
        }; // class Builder
    } // namespace details

    enum Node_t {
        no_type = 0,
        expr = 1,
        decl = 2,
        cond = 3,
        loop = 4, 
        scope = 5,
        output = 6
    };

    enum UnOpNode_t {
        minus = 7,
        negation = 8
    };

    enum BinOpNode_t {
        add = 9,
        sub = 10,
        mul = 11,
        div = 12,
        remainder = 13
    };

    enum BinCompOpNode_t {
        equal = 14,
        not_equal = 15,
        greater = 16,
        less = 17,
        greater_or_equal = 18,
        less_or_equal = 19
    };

    enum LogicOpNode_t {
        logic_and = 20,
        logic_or = 21
    };

    enum ExprNode_t {
        assign = 22,
        logic_op = 23,
        un_op = 24,
        bin_op = 25,
        bin_comp_op = 26,
        number = 27,
        var = 28,
        input = 29
    };

    class NodeInfo {
    public:
        NodeInfo(yy::Location location) : location_(location) {}

        yy::Location &GetLocation() { return location_; }
    private:
        yy::Location location_;
    };

    class NodeVisitor;

    struct Node {
    public:
        Node(Node_t type, yy::Location location) :
            type_(type), info_(location) {}
        virtual ~Node() = default;
        inline virtual void Accept(NodeVisitor &visitor) = 0;
        Node_t type_;
        NodeInfo info_;
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
        std::string GetName() { return name_; }
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
        virtual void visitLogicOpNode(LogicOpNode &node) = 0;
        virtual void visitUnOpNode(UnOpNode &node) = 0;
        virtual void visitBinOpNode(BinOpNode &node) = 0;
        virtual void visitBinCompOpNode(BinCompOpNode &node) = 0;
        virtual void visitNumberNode(NumberNode &node) = 0;
        virtual void visitInputNode(InputNode &node) = 0;
        virtual void visitVarNode(VarNode &node) = 0;
        virtual void visitScopeNode(ScopeNode &node) = 0;
        virtual void visitDeclNode(DeclNode &node) = 0;
        virtual void visitCondNode(CondNode &node) = 0;
        virtual void visitLoopNode(LoopNode &node) = 0;
        virtual void visitAssignNode(AssignNode &node) = 0;
        virtual void visitOutputNode(OutputNode &node) = 0;
    }; // class NodeVisitor

    inline void LogicOpNode::Accept(NodeVisitor &visitor) {
        visitor.visitLogicOpNode(*this);
    }

    inline void UnOpNode::Accept(NodeVisitor &visitor) {
        visitor.visitUnOpNode(*this);
    }

    inline void BinOpNode::Accept(NodeVisitor &visitor) {
        visitor.visitBinOpNode(*this);
    }

    inline void BinCompOpNode::Accept(NodeVisitor &visitor) {
        visitor.visitBinCompOpNode(*this);
    }

    inline void NumberNode::Accept(NodeVisitor &visitor) {
        visitor.visitNumberNode(*this);
    }

    inline void InputNode::Accept(NodeVisitor &visitor) {
        visitor.visitInputNode(*this);
    }

    inline void VarNode::Accept(NodeVisitor &visitor) {
        visitor.visitVarNode(*this);
    }

    inline void ScopeNode::Accept(NodeVisitor &visitor) {
        visitor.visitScopeNode(*this);
    }

    inline void DeclNode::Accept(NodeVisitor &visitor) {
        visitor.visitDeclNode(*this);
    }

    inline void CondNode::Accept(NodeVisitor &visitor) {
        visitor.visitCondNode(*this);
    }

    inline void LoopNode::Accept(NodeVisitor &visitor) {
        visitor.visitLoopNode(*this);
    }

    inline void AssignNode::Accept(NodeVisitor &visitor) {
        visitor.visitAssignNode(*this);
    }

    inline void OutputNode::Accept(NodeVisitor &visitor) {
        visitor.visitOutputNode(*this);
    }
}; // namespace node