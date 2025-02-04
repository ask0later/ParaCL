#pragma once 
#include <vector>
#include <string>

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
        assign = 5,
        scope = 6,
        output = 7
    };

    enum BinOpNode_t {
        add = 8,
        sub = 9,
        mul = 10,
        div = 11
    };

    enum BinCompOpNode_t {
        equal = 12,
        not_equal = 13,
        greater = 14,
        less = 15,
        greater_or_equal = 16,
        less_or_equal = 17
    };

    enum ExprNode_t {
        bin_op = 18,
        bin_comp_op = 19,
        number = 20,
        var = 21,
        input = 22
    };

    class NodeVisitor;

    struct Node {
        Node(Node_t type = no_type) : type_(type) {} 
        virtual ~Node() = default;
        inline virtual void Accept(NodeVisitor &visitor) = 0;
        Node_t type_;
    }; // class Node

    struct ExprNode : public Node {
        ExprNode(ExprNode_t type) : Node(Node_t::expr), type_(type) {}
        inline void Accept(NodeVisitor &visitor) override = 0;
        ExprNode_t type_;
    }; // class ExprNode

    struct BinOpNode final : public ExprNode {
        BinOpNode(BinOpNode_t type, ExprNode *left, ExprNode *right) 
        : ExprNode(ExprNode_t::bin_op), type_(type), left_(left), right_(right) {}
        inline void Accept(NodeVisitor &visitor) override;
        BinOpNode_t type_;
        ExprNode *left_, *right_;
    }; // class BinOpNode

    struct BinCompOpNode final : public ExprNode {
        BinCompOpNode(BinCompOpNode_t type, ExprNode *left, ExprNode *right) 
        : ExprNode(ExprNode_t::bin_comp_op), type_(type), left_(left), right_(right) {}
        inline void Accept(NodeVisitor &visitor) override;
        BinCompOpNode_t type_;
        ExprNode *left_, *right_;
    }; // class BinCompOpNode

    struct NumberNode final : public ExprNode
    {
        NumberNode(int number) : ExprNode(ExprNode_t::number), number_(number) {}
        inline void Accept(NodeVisitor &visitor) override;
        int number_;
    }; // class NumberNode

    struct InputNode final : public ExprNode
    {
        InputNode() : ExprNode(ExprNode_t::input) {}
        inline void Accept(NodeVisitor &visitor) override;
    }; // class InputNode

    struct VarNode final : public ExprNode {
        VarNode(const std::string &name) : ExprNode(ExprNode_t::var), name_(std::move(name)) {}
        inline void Accept(NodeVisitor &visitor) override;
        std::string name_;
    }; // class VarNode

    struct ScopeNode final : public Node {
        ScopeNode() : Node(Node_t::scope), kids_(1) {}
        inline void Accept(NodeVisitor &visitor) override;
        void AddStatement(Node *child) { kids_.push_back(child); }
        std::vector<Node*> kids_;
    }; // class ScopeNode

    struct DeclNode final : public Node {
        DeclNode(const std::string &name) : Node(Node_t::decl), name_(std::move(name)) {}
        inline void Accept(NodeVisitor &visitor) override;
        std::string GetName() { return name_; }
        std::string name_;
    }; // class DeclNode

    struct CondNode final : public Node {
        CondNode(BinCompOpNode *predicat, ScopeNode *first, ScopeNode *second = nullptr) : Node(Node_t::cond), predicat_(predicat), first_(first), second_(second) {}
        inline void Accept(NodeVisitor &visitor) override;
        BinCompOpNode *predicat_;
        ScopeNode *first_;
        ScopeNode *second_;
    }; // class CondNode

    struct LoopNode final : public Node {
        LoopNode(BinCompOpNode *predicat, ScopeNode *scope) : Node(Node_t::loop), predicat_(predicat), scope_(scope) {}
        inline void Accept(NodeVisitor &visitor) override;
        BinCompOpNode *predicat_;
        ScopeNode *scope_;
    }; // class LoopNode 

    struct AssignNode final : public Node {
        AssignNode(DeclNode *var, ExprNode *expr) : Node(Node_t::assign), var_(var), expr_(expr) {}
        inline void Accept(NodeVisitor &visitor) override;
        DeclNode *var_;
        ExprNode *expr_;
    }; // class AssignNode

    struct OutputNode final : public Node {
        OutputNode(ExprNode *expr) : Node(Node_t::output), expr_(expr) {}
        inline void Accept(NodeVisitor &visitor) override;
        ExprNode *expr_;
    }; // class OutputNode

    class NodeVisitor {
    public:
        virtual ~NodeVisitor() = default;
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