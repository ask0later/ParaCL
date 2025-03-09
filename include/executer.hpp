#pragma once
#include <unordered_map>
#include <optional>
#include <exception>
#include <limits>
#include <cassert>
#include <algorithm>

#include "error_handler.hpp"
#include "node.hpp"

namespace executer {
    constexpr size_t DEFAULT_NAME_COUNT = 32; 
    namespace symTable {
        class SymbolTable final {
        public:
            void SetOrAddValue(std::string_view name, int value) {
                varMap_[std::string(name)] = value;
            }

            std::optional<int> GetValue(std::string_view name) const {
                auto name_str = std::string(name);
                
                auto hit = varMap_.find(name_str);
                if (hit != varMap_.end()) 
                    return varMap_.at(name_str);

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

            void SetValue(std::string_view name, int value) {
                auto rbegin = symbolTables_.rbegin();
                auto rend = symbolTables_.rend();
                auto pred = [&name] (const SymbolTable& cur) {
                    auto find = cur.GetValue(name);
                    return find.has_value();
                };
                
                auto it = std::find_if(rbegin, rend, pred);
                if (it != rend) {
                    it->SetOrAddValue(name, value);
                }

                symbolTables_.back().SetOrAddValue(name, value);
            } 

            int GetValue(std::string_view name) const {
                auto rbegin = symbolTables_.rbegin();
                auto rend = symbolTables_.rend();
                auto pred = [&name] (const SymbolTable& cur) {
                    auto find = cur.GetValue(name);
                    return find.has_value();
                };

                auto it = std::find_if(rbegin, rend, pred);
                if (it != rend) {
                    return *(it->GetValue(name));
                }

                throw std::runtime_error(name.data());
            }

            void PushSymTable() {
                symbolTables_.emplace_back();
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

        void Visit(node::LogicOpNode &node) override {
            assert(node.left_);
            node.left_->Accept(*this);
            int operand1 = GetParam();
            assert(node.right_);
            node.right_->Accept(*this);
            int operand2 = GetParam();

            switch (node.type_) {
                case node::LogicOpNode_t::logic_and:
                    SetParam(operand1 && operand2);
                    return;
                case node::LogicOpNode_t::logic_or:
                    SetParam(operand1 || operand2);
                    return;
            }
        }

        void Visit(node::UnOpNode &node) override {
            assert(node.child_);
            node.child_->Accept(*this);

            switch (node.type_) {
                case node::UnOpNode_t::minus:
                    SetParam(-GetParam());
                    return;
                case node::UnOpNode_t::negation:
                    SetParam(!GetParam());
                    return;
            }
        }

        void Visit(node::BinOpNode &node) override {
            assert(node.left_);
            node.left_->Accept(*this);
            int operand1 = GetParam();
            assert(node.right_);
            node.right_->Accept(*this);
            int operand2 = GetParam();

            switch (node.type_) {
                case node::BinOpNode_t::add:
                    SetParam(operand1 + operand2);
                    return;
                case node::BinOpNode_t::sub:
                    SetParam(operand1 - operand2);
                    return;
                case node::BinOpNode_t::mul:
                    SetParam(operand1 * operand2);
                    return;
                case node::BinOpNode_t::div:
                    if (operand2 == 0) {
                        throw std::runtime_error(err_handler_.GetFullErrorMessage("Runtime error", \
                                                                                "Division by zero", \
                                                                                node.location_));
                    }
                    SetParam(operand1 / operand2);
                    return;
                case node::BinOpNode_t::remainder:
                    SetParam(operand1 % operand2);
                    return;  
            }
        }

        void Visit(node::BinCompOpNode &node) override {
            assert(node.left_);
            node.left_->Accept(*this);
            int operand1 = GetParam();
            assert(node.right_);
            node.right_->Accept(*this);
            int operand2 = GetParam();

            switch (node.type_) {
                case node::BinCompOpNode_t::equal:
                    SetParam(operand1 == operand2);
                    return;
                case node::BinCompOpNode_t::not_equal:
                    SetParam(operand1 != operand2);
                    return;
                case node::BinCompOpNode_t::greater:
                    SetParam(operand1 > operand2);
                    return;
                case node::BinCompOpNode_t::less:
                    SetParam(operand1 < operand2);
                    return;
                case node::BinCompOpNode_t::greater_or_equal:
                    SetParam(operand1 >= operand2);
                    return;
                case node::BinCompOpNode_t::less_or_equal:
                    SetParam(operand1 <= operand2);
                    return;
            }
        }

        void Visit(node::NumberNode &node) override {
            SetParam(node.number_);
        }

        void Visit(node::InputNode &node) override {
            int input = 0;
            std::cin >> input;
            SetParam(input);
        }

        void Visit(node::VarNode &node) override {
            try {
                SetParam(symbolTables_.GetValue(node.name_));
            } catch(std::runtime_error& ex) {
                throw std::runtime_error(err_handler_.GetFullErrorMessage("Runtime error", \
                            std::string("'" + std::string(ex.what()) + "' was not declared in this scope"), \
                            node.location_));
            }
        }

        void Visit(node::ScopeNode &node) override {
            symbolTables_.PushSymTable();
            for (auto &statement : node.kids_) {
                if (statement != nullptr)
                    statement->Accept(*this);
            }
            symbolTables_.PopSymTable();
        }

        void Visit(node::DeclNode &node) override {}

        void Visit(node::CondNode &node) override {
            node.predicat_->Accept(*this);

            if (GetParam()) {
                assert(node.first_);
                node.first_->Accept(*this);
            } else {
                if (node.second_)
                    node.second_->Accept(*this);
            }
        }

        void Visit(node::LoopNode &node) override {
            assert(node.predicat_);
            node.predicat_->Accept(*this);
            assert(node.scope_);

            while (GetParam()) {
                node.scope_->Accept(*this);
                node.predicat_->Accept(*this);
            }
        }

        void Visit(node::AssignNode &node) override {
            assert(node.expr_);
            node.expr_->Accept(*this);
            assert(node.var_);
            symbolTables_.SetValue(node.var_->name_, GetParam());
        }

        void Visit(node::OutputNode &node) override {
            assert(node.expr_);
            node.expr_->Accept(*this);
            std::cout << GetParam() << std::endl;
        }

    private:
        int GetParam() const {
            return param_;
        }

        void SetParam(int param) {
            param_ = param;
        }

        int param_ = 0;
        symTable::SymbolTables symbolTables_;
        err::ErrorHandler &err_handler_;
    }; // class ExecuteVisitor
}