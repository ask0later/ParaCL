#pragma once
#include <unordered_map>
#include <optional>
#include <exception>

#include "node.hpp"

namespace executer {
    namespace symTable {
        class SymbolTable final {
        public:
            void SetOrAddValue(std::string &name, int value) {
                varMap_[name] = value;
            }

            std::optional<int> GetValue(std::string &name) {
                auto hit = varMap_.find(name);
                if (hit != varMap_.end()) 
                    return varMap_[name];

                return std::nullopt;
            }

        private:
            std::unordered_map<std::string, int> varMap_;
        };

        class SymbolTables final {
        public:
            SymbolTables() {
                symbolTables_.reserve(16);
            }

            void SetValue(std::string &name, int value) {
                for (auto it = symbolTables_.rbegin(), end = symbolTables_.rend(); it != end; ++it)
                {
                    auto find = it->GetValue(name);
                    if (find.has_value())
                    {
                        it->SetOrAddValue(name, value);
                        return;
                    }
                }
                symbolTables_.back().SetOrAddValue(name, value); // ;(((
            }

            int GetValue(std::string &name) {
                for (auto it = symbolTables_.rbegin(), end = symbolTables_.rend(); it != end; ++it) {
                    auto find = it->GetValue(name);
                    if (find.has_value())
                        return *find;
                }

                std::string res = name;
                res += " was not declared in this scope."; // may be we must find it on compile time
                throw std::logic_error(res);
            }

            void PushSymTable() {
                symbolTables_.push_back(SymbolTable{});
            }

            void PopSymTable() {
                symbolTables_.pop_back();
            }
        private:
            std::vector<SymbolTable> symbolTables_;
        };
    }; // namespace symTable

    class ExecuteVisitor final : public node::NodeVisitor {
    public:
        void visitBinOpNode(node::BinOpNode &node) override {
            node.left_->Accept(*this);
            int operand1 = int_param_;
            node.right_->Accept(*this);
            int operand2 = int_param_;

            switch (node.type_) {
                case node::BinOpNode_t::add:
                    int_param_ = operand1 + operand2;
                    return;
                case node::BinOpNode_t::sub:
                    int_param_ = operand1 - operand2;
                    return;
                case node::BinOpNode_t::mul:
                    int_param_ = operand1 * operand2;
                    return;
                case node::BinOpNode_t::div:
                    if (operand2 == 0) {
                        // runtime error
                    }
                    int_param_ = operand1 / operand2;
                    return;
            }
        }

        void visitBinCompOpNode(node::BinCompOpNode &node) override {
            node.left_->Accept(*this);
            int operand1 = int_param_;
            node.right_->Accept(*this);
            int operand2 = int_param_;

            switch (node.type_) {
                case node::BinCompOpNode_t::equal:
                    bool_param_ = (operand1 == operand2);
                    return;
                case node::BinCompOpNode_t::not_equal:
                    bool_param_ = (operand1 != operand2);
                    return;
                case node::BinCompOpNode_t::greater:
                    bool_param_ = (operand1 > operand2);
                    return;
                case node::BinCompOpNode_t::less:
                    bool_param_ = (operand1 < operand2);
                    return;
                case node::BinCompOpNode_t::greater_or_equal:
                    bool_param_ = (operand1 >= operand2);
                    return;
                case node::BinCompOpNode_t::less_or_equal:
                    bool_param_ = (operand1 <= operand2);
                    return;
            }
        }
        void visitNumberNode(node::NumberNode &node) override {
            int_param_ = node.number_;
        }
        void visitInputNode(node::InputNode &node) override {
            int input = 0;
            std::cin >> input;
            int_param_ = input;
        }
        void visitVarNode(node::VarNode &node) override {
            int_param_ = symbolTables_.GetValue(node.name_);
        }
        void visitScopeNode(node::ScopeNode &node) override {
            symbolTables_.PushSymTable();
            for (auto &statement : node.kids_) {
                if (statement != nullptr)
                    statement->Accept(*this);
            }
            symbolTables_.PopSymTable();
        }
        void visitDeclNode(node::DeclNode &node) override {
            string_param_ = node.name_;
        }
        void visitCondNode(node::CondNode &node) override {
            node.predicat_->Accept(*this);
            std::cerr << bool_param_ << std::endl;
            if (bool_param_) {
                node.first_->Accept(*this);
            } else {
                if (node.second_)
                    node.second_->Accept(*this);
            }
        }
        void visitLoopNode(node::LoopNode &node) override {
            node.predicat_->Accept(*this);
            while (bool_param_) {
                node.scope_->Accept(*this);
                node.predicat_->Accept(*this);
            }
        }
        void visitAssignNode(node::AssignNode &node) override {
            node.expr_->Accept(*this);
            int result = int_param_;
            node.var_->Accept(*this);
            auto name = string_param_;
            symbolTables_.SetValue(name, result);
        }
        void visitOutputNode(node::OutputNode &node) override {
            node.expr_->Accept(*this);
            int res = int_param_; 
            std::cout << res << std::endl;
        }

    private:
        bool bool_param_ = false;
        int int_param_ = 0;
        std::string string_param_;
        symTable::SymbolTables symbolTables_;
    }; // class ExecuteVisitor
}