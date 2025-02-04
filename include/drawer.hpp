#pragma once
#include <cassert>

#include "dotter.hpp"
#include "node.hpp"

namespace drawer {
    const static std::map<int, std::string> OpTexts = {
        { node::BinOpNode_t::add, "+" },
        { node::BinOpNode_t::sub, "-" },
        { node::BinOpNode_t::mul, "*" },
        { node::BinOpNode_t::div, "/" },

        { node::BinCompOpNode_t::equal,            "==" },
        { node::BinCompOpNode_t::not_equal,        "!=" },
        { node::BinCompOpNode_t::greater,          ">"  },
        { node::BinCompOpNode_t::greater_or_equal, ">=" },
        { node::BinCompOpNode_t::less,             "<"  },
        { node::BinCompOpNode_t::less_or_equal,    "<=" }
    };

    class DrawVisitor final : public node::NodeVisitor {
    public:
        DrawVisitor(dotter::Dotter &dotter) : dotter_(dotter) { }
        
        void visitBinOpNode(node::BinOpNode &node) override {
            dotter_.SetNodeStyle(dotter::NodeStyle::SHAPES::BOX, dotter::NodeStyle::STYLES::BOLD,
                                  dotter::COLORS::BLACK, dotter::COLORS::RED, dotter::COLORS::BLACK);
            
            dotter_.AddNode(OpTexts.at(node.type_), reinterpret_cast<std::size_t>(std::addressof(node)));

            if (node.left_ != nullptr) {
                node.left_->Accept(*this);
                dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.left_));
            }

            if (node.right_ != nullptr) {
                node.right_->Accept(*this);
                dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.right_));
            }
        }

        void visitBinCompOpNode(node::BinCompOpNode &node) override {
            dotter_.SetNodeStyle(dotter::NodeStyle::SHAPES::BOX, dotter::NodeStyle::STYLES::BOLD,
                                  dotter::COLORS::BLACK, dotter::COLORS::RED, dotter::COLORS::BLACK);
            
            dotter_.AddNode(OpTexts.at(node.type_), reinterpret_cast<std::size_t>(std::addressof(node)));
            if (node.left_ != nullptr) {
                node.left_->Accept(*this);
                dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.left_));
            }

            if (node.right_ != nullptr) {
                node.right_->Accept(*this);
                dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.right_));
            }
        }
        void visitNumberNode(node::NumberNode &node) override {
            dotter_.SetNodeStyle(dotter::NodeStyle::SHAPES::DIAMOND, dotter::NodeStyle::STYLES::BOLD,
                                  dotter::COLORS::BLACK, dotter::COLORS::BLUE, dotter::COLORS::WHITE);
            dotter_.AddNode(std::to_string(node.number_), reinterpret_cast<std::size_t>(std::addressof(node)));
        }
        void visitInputNode(node::InputNode &node) override {
            dotter_.SetNodeStyle(dotter::NodeStyle::SHAPES::TRIANGLE, dotter::NodeStyle::STYLES::BOLD,
                                  dotter::COLORS::BLACK, dotter::COLORS::YELLOW, dotter::COLORS::BLACK);
            dotter_.AddNode("Input", reinterpret_cast<std::size_t>(std::addressof(node)));
        }
        void visitVarNode(node::VarNode &node) override {
            dotter_.SetNodeStyle(dotter::NodeStyle::SHAPES::DIAMOND, dotter::NodeStyle::STYLES::BOLD,
                                  dotter::COLORS::BLACK, dotter::COLORS::GREEN, dotter::COLORS::BLACK);
            dotter_.AddNode(node.name_, reinterpret_cast<std::size_t>(std::addressof(node)));
        }
        void visitScopeNode(node::ScopeNode &node) override {
            dotter_.SetNodeStyle(dotter::NodeStyle::SHAPES::ELLIPSE, dotter::NodeStyle::STYLES::BOLD,
                                  dotter::COLORS::BLACK, dotter::COLORS::WHITE, dotter::COLORS::BLACK);
            dotter_.AddNode("Scope", reinterpret_cast<std::size_t>(std::addressof(node)));
            for (size_t i = 0, kids_size = node.kids_.size() ; i < kids_size; ++i) {
                if (node.kids_[i] == nullptr)
                    continue;
                node.kids_[i]->Accept(*this);
                dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.kids_[i]));
            }
        }
        void visitDeclNode(node::DeclNode &node) override {
            dotter_.SetNodeStyle(dotter::NodeStyle::SHAPES::ELLIPSE, dotter::NodeStyle::STYLES::BOLD,
                                  dotter::COLORS::BLACK, dotter::COLORS::GREEN, dotter::COLORS::BLACK);
            dotter_.AddNode(node.name_, reinterpret_cast<std::size_t>(std::addressof(node)));
        }
        void visitCondNode(node::CondNode &node) override {
            dotter_.SetNodeStyle(dotter::NodeStyle::SHAPES::ELLIPSE, dotter::NodeStyle::STYLES::BOLD,
                                  dotter::COLORS::BLACK, dotter::COLORS::BLUE, dotter::COLORS::WHITE);
            dotter_.AddNode("If", reinterpret_cast<std::size_t>(std::addressof(node)));
            if (node.predicat_ != nullptr) {
                node.predicat_->Accept(*this);
                dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.predicat_));
            }

            if (node.first_ != nullptr) {
                node.first_->Accept(*this);
                dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.first_));
            }

            if (node.second_ != nullptr) {
                node.second_->Accept(*this);
                dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.second_));
            }
        }
        
        void visitLoopNode(node::LoopNode &node) override {
            dotter_.SetNodeStyle(dotter::NodeStyle::SHAPES::ELLIPSE, dotter::NodeStyle::STYLES::BOLD,
                                  dotter::COLORS::BLACK, dotter::COLORS::BLUE, dotter::COLORS::WHITE);
            dotter_.AddNode("While", reinterpret_cast<std::size_t>(std::addressof(node)));

            if (node.predicat_ == nullptr)
                std::terminate();
            
            node.predicat_->Accept(*this);
            dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.predicat_));

            assert(node.scope_);
            node.scope_->Accept(*this);
            dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.scope_));
        }

        void visitAssignNode(node::AssignNode &node) override {
            dotter_.SetNodeStyle(dotter::NodeStyle::SHAPES::BOX, dotter::NodeStyle::STYLES::BOLD,
                                  dotter::COLORS::BLACK, dotter::COLORS::BLUE, dotter::COLORS::WHITE);
            dotter_.AddNode("=", reinterpret_cast<std::size_t>(std::addressof(node)));
            if (node.var_ != nullptr) {
                node.var_->Accept(*this);
                dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.var_)); 
            }

            if (node.expr_ != nullptr) {
                node.expr_->Accept(*this);
                dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.expr_));
            }
        }
        
        void visitOutputNode(node::OutputNode &node) override {
            dotter_.SetNodeStyle(dotter::NodeStyle::SHAPES::TRIANGLE, dotter::NodeStyle::STYLES::BOLD,
                                  dotter::COLORS::BLACK, dotter::COLORS::YELLOW, dotter::COLORS::BLACK);
            dotter_.AddNode("Output", reinterpret_cast<std::size_t>(std::addressof(node)));
            if (node.expr_ != nullptr) {
                node.expr_->Accept(*this);
                dotter_.AddLink(reinterpret_cast<std::size_t>(std::addressof(node)), reinterpret_cast<std::size_t>(node.expr_));
            }
        }
    private:
        dotter::Dotter &dotter_;
    }; // class DrawVisitor
}; // namespace drawer