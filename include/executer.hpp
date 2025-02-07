#pragma once
#include <unordered_map>
#include <optional>
#include <exception>
#include <limits>
#include <cassert>

#include "error_handler.hpp"
#include "node.hpp"

namespace executer {
    constexpr size_t DEFAULT_NAME_COUNT = 32; 
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
                symbolTables_.reserve(DEFAULT_NAME_COUNT);
            }

            void SetValue(std::string &name, int value) {
                for (auto it = symbolTables_.rbegin(), end = symbolTables_.rend(); it != end; ++it) {
                    auto find = it->GetValue(name);
                    if (find.has_value()) {
                        it->SetOrAddValue(name, value);
                        return;
                    }
                }
                symbolTables_.back().SetOrAddValue(name, value);
            } 

            int GetValue(std::string &name) {
                for (auto it = symbolTables_.rbegin(), end = symbolTables_.rend(); it != end; ++it) {
                    auto find = it->GetValue(name);
                    if (find.has_value())
                        return *find;
                }

                throw std::runtime_error(name);
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
        ExecuteVisitor(err::ErrorHandler &err_handler) : err_handler_(err_handler) {}

        void visitUnOpNode(node::UnOpNode &node) override {
            assert(node.child_);
            node.child_->Accept(*this);

            switch (node.type_) {
                case node::UnOpNode_t::minus:
                    param_ = - param_;
                    return;
                case node::UnOpNode_t::negation:
                    param_ = !(param_);
                    return;
            }
        }

        void visitBinOpNode(node::BinOpNode &node) override {
            assert(node.left_);
            node.left_->Accept(*this);
            int operand1 = param_;
            assert(node.right_);
            node.right_->Accept(*this);
            int operand2 = param_;

            switch (node.type_) {
                case node::BinOpNode_t::add:
                    param_ = operand1 + operand2;
                    return;
                case node::BinOpNode_t::sub:
                    param_ = operand1 - operand2;
                    return;
                case node::BinOpNode_t::mul:
                    param_ = operand1 * operand2;
                    return;
                case node::BinOpNode_t::div:
                    if (operand2 == 0) {
                        throw std::runtime_error(err_handler_.GetFullErrorMessage("Runtime error", \
                                                                                "Division by zero", \
                                                                                node.info_.GetNumLine()));
                    }
                    param_ = operand1 / operand2;
                    return;
                case node::BinOpNode_t::remainder:
                    param_ = operand1 % operand2;
                    return;  
            }
        }

        void visitBinCompOpNode(node::BinCompOpNode &node) override {
            assert(node.left_);
            node.left_->Accept(*this);
            int operand1 = param_;
            assert(node.right_);
            node.right_->Accept(*this);
            int operand2 = param_;

            switch (node.type_) {
                case node::BinCompOpNode_t::equal:
                    param_ = (operand1 == operand2);
                    return;
                case node::BinCompOpNode_t::not_equal:
                    param_ = (operand1 != operand2);
                    return;
                case node::BinCompOpNode_t::greater:
                    param_ = (operand1 > operand2);
                    return;
                case node::BinCompOpNode_t::less:
                    param_ = (operand1 < operand2);
                    return;
                case node::BinCompOpNode_t::greater_or_equal:
                    param_ = (operand1 >= operand2);
                    return;
                case node::BinCompOpNode_t::less_or_equal:
                    param_ = (operand1 <= operand2);
                    return;
            }
        }

        void visitNumberNode(node::NumberNode &node) override {
            param_ = node.number_;
        }

        void visitInputNode(node::InputNode &node) override {
            int input = 0;
            std::cin >> input;
            param_ = input;
        }

        void visitVarNode(node::VarNode &node) override {
            try {
                param_ = symbolTables_.GetValue(node.name_);
            } catch(std::runtime_error& ex) {
                throw std::runtime_error(err_handler_.GetFullErrorMessage("Runtime error", \
                            std::string("'" + std::string(ex.what()) + "' was not declared in this scope"), \
                            node.info_.GetNumLine()));
            }
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
            if (param_) {
                assert(node.first_);
                node.first_->Accept(*this);
            } else {
                if (node.second_)
                    node.second_->Accept(*this);
            }
        }

        void visitLoopNode(node::LoopNode &node) override {
            assert(node.predicat_);
            node.predicat_->Accept(*this);
            assert(node.scope_);
            while (param_) {
                node.scope_->Accept(*this);
                node.predicat_->Accept(*this);
            }
        }

        void visitAssignNode(node::AssignNode &node) override {
            assert(node.expr_);
            node.expr_->Accept(*this);
            int result = param_;

            assert(node.var_);
            node.var_->Accept(*this);
            auto name = string_param_;
            symbolTables_.SetValue(name, result);
            param_ = 1;
        }

        void visitOutputNode(node::OutputNode &node) override {
            assert(node.expr_);
            node.expr_->Accept(*this);
            int res = param_; 
            std::cout << res << std::endl;
        }

    private:
        //bool bool_param_ = false;
        int param_ = 0;
        std::string string_param_;
        symTable::SymbolTables symbolTables_;
        err::ErrorHandler &err_handler_;
    }; // class ExecuteVisitor
}