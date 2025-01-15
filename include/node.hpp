#pragma once 

#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>


namespace node {

    enum Node_t
    {
        no_type = 0,
        expr = 1,
        decl = 2,
        cond = 3,
        loop = 4, 
        assign = 5,
        scope = 6,
        output = 7
    };

    enum BinOpNode_t
    {
        add = 8,
        sub = 9,
        mul = 10,
        div = 11
    };

    enum BinCompOpNode_t
    {
        equal = 12,
        not_equal = 13,
        greater = 14,
        less = 15,
        greater_or_equal = 16,
        less_or_equal = 17
    };

    enum ExprNode_t
    {
        bin_op = 18,
        bin_comp_op = 19,
        number = 20,
        var = 21,
        input = 22
    };

    class SymbolTable final {
    public:
        void SetOrAddName(std::string &name, int value) {
            varMap_[name] = value;
        }

        int GetValue(std::string &name) {
            auto hit = varMap_.find(name);
            if (hit != varMap_.end()) 
                return varMap_[name];

            std::cout << "Error" << std::endl;
            std::terminate();
        }

    private:
        std::unordered_map<std::string, int> varMap_;
    };
    
    class Node {
    public:
        Node(Node_t type = no_type) : type_(type) {} 
        virtual ~Node() {}
        virtual int Execute() = 0;
        // virtual void GetInfo() = 0;

        SymbolTable symbolTable_{};
    private:
        Node_t type_;
    }; // class Node

    // SymbolTable Node::symbolTable_{};

    class ExprNode : public Node 
    {
    public:
        ExprNode(ExprNode_t type) : Node(Node_t::expr), type_(type) {}
        int Execute() override = 0;
        // void GetInfo() override = 0;
    private:
        ExprNode_t type_;
    }; // class ExprNode

    class BinOpNode final : public ExprNode
    {
    public:
        BinOpNode(BinOpNode_t type, ExprNode *left, ExprNode *right) 
        : ExprNode(ExprNode_t::bin_op), type_(type), left_(left), right_(right) {}
        int Execute() override {
            int operand1 = left_->Execute(), operand2 = right_->Execute();

            switch (type_) {
                case BinOpNode_t::add:
                    return operand1 + operand2;
                case BinOpNode_t::sub:
                    return operand1 - operand2;
                case BinOpNode_t::mul:
                    return operand1 * operand2;
                case BinOpNode_t::div:
                    return operand1 / operand2;
            }
            return 0;
        }

        // BinOpNode_t GetInfo() override {
        //     return type_;
        // }
    private:
        BinOpNode_t type_;
        ExprNode *left_, *right_;
    }; // class BinOpNode

    class BinCompOpNode final : public ExprNode
    {
    public:
        BinCompOpNode(BinCompOpNode_t type, ExprNode *left, ExprNode *right) 
        : ExprNode(ExprNode_t::bin_comp_op), type_(type), left_(left), right_(right) {}
        int Execute() override {
            int operand1 = left_->Execute(), operand2 = right_->Execute();

            switch (type_) {
                case BinCompOpNode_t::equal:
                    return operand1 == operand2;
                case BinCompOpNode_t::not_equal:
                    return operand1 != operand2;
                case BinCompOpNode_t::greater:
                    return operand1 >  operand2;
                case BinCompOpNode_t::less:
                    return operand1 <  operand2;
                case BinCompOpNode_t::greater_or_equal:
                    return operand1 >= operand2;
                case BinCompOpNode_t::less_or_equal:
                    return operand1 <= operand2;
            }

            return 0;
        }

        // BinCompOpNode_t GetInfo() override {
        //     return type_;
        // }
    private:
        BinCompOpNode_t type_;
        ExprNode *left_, *right_;
    }; // class BinCompOpNode

    class NumberNode final : public ExprNode
    {
    public:
        NumberNode(int number) : ExprNode(ExprNode_t::number), number_(number) {}
        int Execute() override {
            return number_;
        }

        // ExprNode_t GetInfo() override {
        //     return ExprNode_t::number;
        // }
    private:
        int number_;
    }; // class NumberNode

    class InputNode final : public ExprNode
    {
    public:
        InputNode() : ExprNode(ExprNode_t::input) {}
        int Execute() override {
            int input = 0;
            std::cin >> input;
            return input; 
        }

        // ExprNode_t GetInfo() override {
        //     return ExprNode_t::input;
        // }
    }; // class InputNode

    class VarNode final : public ExprNode
    {
    public:
        VarNode(const std::string &name) : ExprNode(ExprNode_t::var), name_(std::move(name)) {}
        
        int Execute() override {
            return symbolTable_.GetValue(name_);
        }

        // ExprNode_t GetInfo() override {
        //     return ExprNode_t::var;
        // }
    private:
        std::string name_;
    }; // class VarNode

    class ScopeNode final : public Node
    {
    public:
        ScopeNode() : Node(Node_t::scope), kids_(1) {}
        void AddStatement(Node *child) { kids_.push_back(child); }
        
        int Execute() override {
            for (auto &statement : kids_) {
                statement->Execute();
            }
            return 0;
        }

        // Node_t GetInfo() override {
        //     return Node_t::decl;
        // }
    private:
        std::vector<Node*> kids_;
    }; // class ScopeNode

    class DeclNode final : public Node
    {
    public:
        DeclNode(const std::string &name) : Node(Node_t::decl), name_(std::move(name)) {}

        // std::string Execute() override {
        //     return name_;
        // }

        int Execute() override {
            return 0;
        }

        std::string GetName() {
            return name_;
        }
        // Node_t GetInfo() override {
        //     return Node_t::decl;
        // }
    private:
        std::string name_;
    }; // class DeclNode

    class CondNode final : public Node 
    {
    public:
        CondNode(BinCompOpNode *predicat, ScopeNode *first, ScopeNode *second = nullptr) : Node(Node_t::cond), predicat_(predicat), first_(first), second_(second) {}

        int Execute() override {
            if (predicat_->Execute()) {
                first_->Execute();
            } else {
                second_->Execute();
            }
            return 0;
        }

        // Node_t GetInfo() override {
        //     return Node_t::cond;
        // }
    private:
        BinCompOpNode *predicat_;
        ScopeNode *first_;
        ScopeNode *second_;
    }; // class CondNode

    class LoopNode final : public Node
    {
    public:
        LoopNode(BinCompOpNode *predicat, ScopeNode *scope) : Node(Node_t::loop), predicat_(predicat_), scope_(scope) {}

        int Execute() override {
            while (predicat_->Execute()) {
                scope_->Execute();
            }
            return 0;
        }

        // Node_t GetInfo() override {
        //     return Node_t::loop;
        // }
    private:
        BinCompOpNode *predicat_;
        ScopeNode *scope_;
    }; // class LoopNode 

    class AssignNode final : public Node
    {
    public:
        AssignNode(DeclNode *var, ExprNode *expr) : Node(Node_t::assign), var_(var), expr_(expr) {}

        int Execute() override {
            int result = expr_->Execute();
            auto name = var_->GetName();
            symbolTable_.SetOrAddName(name, result);
            return 0;
        }
        
        // Node_t GetInfo() override {
        //     return Node_t::assign;
        // }

    private:
        DeclNode *var_;
        ExprNode *expr_;
    }; // class AssignNode

    class OutputNode final : public Node
    {
    public:
        OutputNode(ExprNode *expr) : Node(Node_t::output), expr_(expr) {}

        int Execute() override {
            std::cout << expr_->Execute() << std::endl;
            return 0;
        }

        // Node_t GetInfo() override {
        //     return Node_t::output;
        // }
    private:
        ExprNode *expr_;
    }; // class OutputNode

}; // namespace node
