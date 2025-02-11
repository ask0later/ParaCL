#pragma once

#include <map>
#include <cstring>
#include <fstream>
#include <cstdlib>

namespace dotter {
    enum COLORS {
        BLACK   = 0,
        WHITE   = 1,
        RED     = 2,
        BLUE    = 3,
        YELLOW  = 4,
        GREEN   = 5
    }; // enum COLORS
    
    class NodeStyle {
    public:
        enum SHAPES {
            BOX         = 0,
            ELLIPSE     = 1,
            TRIANGLE    = 2,
            DIAMOND     = 3
        }; // enum SHAPES

        enum STYLES {
            SOLID   = 0,
            ROUNDED = 1,
            BOLD    = 3
        }; // enum STYLES

        NodeStyle(SHAPES shape, STYLES style, COLORS color, COLORS fill_color, COLORS font_color) :
        shape_(shape), style_(style), color_(color), fill_color_(fill_color), font_color_(font_color) {}

        NodeStyle() {}

        void SetStyle(SHAPES shape, STYLES style, COLORS color, COLORS fill_color, COLORS font_color) {
            shape_      = shape;
            style_      = style;
            color_      = color;
            fill_color_ = fill_color;
            font_color_ = font_color;
        }

    public:
        SHAPES shape_       = SHAPES::BOX;
        STYLES style_       = STYLES::ROUNDED;
        COLORS color_       = COLORS::BLACK;
        COLORS fill_color_  = COLORS::WHITE;
        COLORS font_color_  = COLORS::BLACK;

    }; // class NodeStyle

    class LinkStyle {
    public:
        enum STYLES {
            SOLID   = 0,
            DASHED  = 1,
            BOLD    = 2
        }; // enum STYLES

        LinkStyle(STYLES style, COLORS color) : style_(style), color_(color) {}

        LinkStyle() {}

        void SetStyle(STYLES style, COLORS color) {
            style_ = style;
            color_ = color;
        }

    public:
        STYLES style_ = STYLES::BOLD;
        COLORS color_ = COLORS::BLACK;
    }; // class LinkStyle

    class Node {
    public:
        Node(NodeStyle &style, std::string text, std::size_t id) : style_(style), text_(text), id_(id) {}

    public:
        NodeStyle style_;
        std::string text_;
        std::size_t id_;
    }; // class Node

    class Link
    {
    public:
        Link(LinkStyle &style, std::string text, std::size_t node1_id, std::size_t node2_id) :
        style_(style), text_(text), node1_id_(node1_id), node2_id_(node2_id) {}
    public:
        LinkStyle style_;
        std::string text_;
        std::size_t node1_id_;
        std::size_t node2_id_;
    }; // class Link

    static std::map<COLORS, std::string> colors_map = {
        {COLORS::RED,    "red"   },
        {COLORS::BLUE,   "blue"  },
        {COLORS::BLACK,  "black" },
        {COLORS::GREEN,  "green" },
        {COLORS::WHITE,  "white" },
        {COLORS::YELLOW, "yellow"}
    };

    static std::map<NodeStyle::SHAPES, std::string> node_shapes_map = {
        {NodeStyle::SHAPES::BOX,      "box"     },
        {NodeStyle::SHAPES::DIAMOND,  "diamond" },
        {NodeStyle::SHAPES::ELLIPSE,  "ellipse" },
        {NodeStyle::SHAPES::TRIANGLE, "triangle"}
    };

    static std::map<NodeStyle::STYLES, std::string> node_styles_map = {
        {NodeStyle::STYLES::BOLD,    "bold"   },
        {NodeStyle::STYLES::ROUNDED, "rounded"},
        {NodeStyle::STYLES::SOLID,   "solid"  }
    };

    static std::map<LinkStyle::STYLES, std::string> link_styles_map = {
        {LinkStyle::STYLES::BOLD,   "bold"  },
        {LinkStyle::STYLES::DASHED, "dashed"},
        {LinkStyle::STYLES::SOLID,  "solid" }
    };

    class Dotter {
    public:
        Dotter() {
            current_dot_ += "digraph DotGraph\n{\n";
        }

        void SetNodeStyle(NodeStyle::SHAPES shape, NodeStyle::STYLES style,
                            COLORS color, COLORS fill_color, COLORS font_color) {
            current_node_style_.SetStyle(shape, style, color, fill_color, font_color);
        }

        void SetLinkStyle(LinkStyle::STYLES style, COLORS color) {
            current_link_style_.SetStyle(style, color);
        }

        void AddNode(std::string text, std::size_t id) {
            nodes_.push_back({current_node_style_, text, id});
        }

        void AddNode(std::string text, std::size_t id, NodeStyle style) {
            nodes_.push_back({style, text, id});
        }

        void AddNode(std::string text, std::size_t id,
                      NodeStyle::SHAPES shape, NodeStyle::STYLES style,
                      COLORS color, COLORS fill_color, COLORS font_color) {
            NodeStyle new_style{shape, style, color, fill_color, font_color};
            nodes_.push_back({new_style, text, id});
        }

        void AddLink(std::size_t node1_id, std::size_t node2_id) {
            links_.push_back({current_link_style_, "", node1_id, node2_id});
        }

        void AddLink(std::size_t node1_id, std::size_t node2_id, LinkStyle style) {
            links_.push_back({style, "", node1_id, node2_id});
        }

        void AddLink(std::string text, std::size_t node1_id, std::size_t node2_id) {
            links_.push_back({current_link_style_, text, node1_id, node2_id});
        }

        void AddLink(std::string text, std::size_t node1_id, std::size_t node2_id,
                      LinkStyle::STYLES style, COLORS color) {
            LinkStyle new_style{style, color};
            links_.push_back({new_style, text, node1_id, node2_id});
        }

        void AddLink(std::size_t node1_id, std::size_t node2_id,
                      LinkStyle::STYLES style, COLORS color) {
            LinkStyle new_style{style, color};
            links_.push_back({new_style, "", node1_id, node2_id});
        }

        void PrintDotText(std::ostream& stream) {
            GenerateDotText();
            stream << current_dot_;
        }

        void PrintDotText(std::string dot_file_name = "graph.dot") {
            GenerateDotText();
            std::ofstream out;
            out.open(dot_file_name);
            out << current_dot_;
            out.close();
        }

        void Render(const char* dot_file_name = "graph.dot", const char* image_file_name = "graph.png") {
            char str1[100] = "dot -Tpng ";
            char str2[] = " -o ";
            strcat(str1, dot_file_name);
            strcat(str1, str2);
            strcat(str1, image_file_name);
            int ret = std::system(str1);
            (void)ret;
        }
        
    private:
        void GenerateDotText() {
            for (auto& node : nodes_) {
                current_dot_ += "\tNode" + std::to_string(node.id_) + " [";
                current_dot_ += "shape=\"" + node_shapes_map[node.style_.shape_] + "\", ";
                current_dot_ += "color=\"" + colors_map[node.style_.color_] + "\", ";
                current_dot_ += "fontcolor=\"" + colors_map[node.style_.font_color_] + "\", ";
                current_dot_ += "fillcolor=\"" + colors_map[node.style_.fill_color_] + "\", ";
                current_dot_ += "style=\"" + node_styles_map[node.style_.style_] + ", filled\", ";
                current_dot_ += "weight=\"1\", ";
                current_dot_ += "label=\"" + node.text_ + "\"";
                current_dot_ += "];\n";
            } 

            for (auto& link : links_) {
                current_dot_ += "\tNode" + std::to_string(link.node1_id_) + " -> " +
                                "Node" + std::to_string(link.node2_id_) + " [";
                current_dot_ += "color=\"" + colors_map[link.style_.color_] + "\", ";
                current_dot_ += "style=\"" + link_styles_map[link.style_.style_] + ", filled\", ";
                current_dot_ += "weight=\"1\", ";
                current_dot_ += "label=\"" + link.text_ + "\"";
                current_dot_ += "];\n";
            }

            current_dot_ += "}\n";
        }

    private:
        std::string current_dot_;
        NodeStyle   current_node_style_;
        LinkStyle   current_link_style_;
        std::vector<Node> nodes_;
        std::vector<Link> links_;
    }; // class Dotter
} // namespace dotter