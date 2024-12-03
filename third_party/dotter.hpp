#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstring>

namespace dotter
{
    enum COLORS
    {
        BLACK   = 0,
        WHITE   = 1,
        RED     = 2,
        BLUE    = 3,
        YELLOW  = 4,
        GREEN   = 5
    }; // enum COLORS
    
    class NodeStyle
    {
public:
        enum SHAPES
        {
            BOX         = 0,
            ELLIPSE     = 1,
            TRIANGLE    = 2,
            DIAMOND     = 3
        }; // enum SHAPES

        enum STYLES
        {
            SOLID   = 0,
            ROUNDED = 1,
            BOLD    = 3
        }; // enum STYLES

        NodeStyle(SHAPES shape, STYLES style, COLORS color, COLORS fill_color, COLORS font_color) :
        shape_(shape), style_(style), color_(color), fill_color_(fill_color), font_color_(font_color) {}

        NodeStyle() {}

        void set_style(SHAPES shape, STYLES style, COLORS color, COLORS fill_color, COLORS font_color)
        {
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

    class LinkStyle
    {
public:
        enum STYLES
        {
            SOLID   = 0,
            DASHED  = 1,
            BOLD    = 2
        }; // enum STYLES

        LinkStyle(STYLES style, COLORS color) : style_(style), color_(color) {}

        LinkStyle() {}

        void set_style(STYLES style, COLORS color)
        {
            style_ = style;
            color_ = color;
        }

public:
        STYLES style_ = STYLES::BOLD;
        COLORS color_ = COLORS::BLACK;
    }; // class LinkStyle

    class Node
    {
public:
        Node(NodeStyle &style, std::string text, int id) : style_(style), text_(text), id_(id) {}

public:
        NodeStyle style_;
        std::string text_;
        int id_;
    }; // class Node

    class Link
    {
public:
        Link(LinkStyle &style, std::string text, int node1_id, int node2_id) :
        style_(style), text_(text), node1_id_(node1_id), node2_id_(node2_id) {}

public:
        LinkStyle style_;
        std::string text_;
        int node1_id_;
        int node2_id_;
    }; // class Link

    static std::map<COLORS, std::string> colors_map = 
    {
        {COLORS::BLACK, "black"},
        {COLORS::BLUE, "blue"},
        {COLORS::GREEN, "green"},
        {COLORS::RED, "red"},
        {COLORS::WHITE, "white"},
        {COLORS::YELLOW, "yellow"}
    };

    static std::map<NodeStyle::SHAPES, std::string> node_shapes_map = 
    {
        {NodeStyle::SHAPES::BOX, "box"},
        {NodeStyle::SHAPES::DIAMOND, "diamond"},
        {NodeStyle::SHAPES::ELLIPSE, "ellipse"},
        {NodeStyle::SHAPES::TRIANGLE, "triangle"}
    };

    static std::map<NodeStyle::STYLES, std::string> node_styles_map = 
    {
        {NodeStyle::STYLES::BOLD, "bold"},
        {NodeStyle::STYLES::ROUNDED, "rounded"},
        {NodeStyle::STYLES::SOLID, "solid"}
    };

    static std::map<LinkStyle::STYLES, std::string> link_styles_map = 
    {
        {LinkStyle::STYLES::BOLD, "bold"},
        {LinkStyle::STYLES::DASHED, "dashed"},
        {LinkStyle::STYLES::SOLID, "solid"}
    };

    class Dotter
    {
public:
        Dotter()
        {
            current_dot_ += "digraph DotGraph\n{\n";
        }

        void set_node_style(NodeStyle::SHAPES shape, NodeStyle::STYLES style,
                            COLORS color, COLORS fill_color, COLORS font_color)
        {
            current_node_style_.set_style(shape, style, color, fill_color, font_color);
        }

        void set_link_style(LinkStyle::STYLES style, COLORS color)
        {
            current_link_style_.set_style(style, color);
        }

        void add_node(std::string text, int id)
        {
            nodes_.push_back({current_node_style_, text, id});
        }

        void add_node(std::string text, int id, NodeStyle style)
        {
            nodes_.push_back({style, text, id});
        }

        void add_node(std::string text, int id,
                      NodeStyle::SHAPES shape, NodeStyle::STYLES style,
                      COLORS color, COLORS fill_color, COLORS font_color)
        {
            NodeStyle new_style{shape, style, color, fill_color, font_color};
            nodes_.push_back({new_style, text, id});
        }

        void add_link(int node1_id, int node2_id)
        {
            links_.push_back({current_link_style_, "", node1_id, node2_id});
        }

        void add_link(int node1_id, int node2_id, LinkStyle style)
        {
            links_.push_back({style, "", node1_id, node2_id});
        }

        void add_link(std::string text, int node1_id, int node2_id)
        {
            links_.push_back({current_link_style_, text, node1_id, node2_id});
        }

        void add_link(std::string text, int node1_id, int node2_id,
                      LinkStyle::STYLES style, COLORS color)
        {
            LinkStyle new_style{style, color};
            links_.push_back({new_style, text, node1_id, node2_id});
        }

        void add_link(int node1_id, int node2_id,
                      LinkStyle::STYLES style, COLORS color)
        {
            LinkStyle new_style{style, color};
            links_.push_back({new_style, "", node1_id, node2_id});
        }

        void print_dot_text(std::ostream& stream)
        {
            generate_dot_text();
            stream << current_dot_;
        }

        void print_dot_text(std::string dot_file_name="graph.dot")
        {
            generate_dot_text();
            std::ofstream out;
            out.open(dot_file_name);
            out << current_dot_;
            out.close();
        }

        void render(const char* dot_file_name="graph.dot", const char* image_file_name="graph.png")
        {
            char str1[100] = "dot -Tpng ";
            char str2[] = " -o ";
            strcat(str1, dot_file_name);
            strcat(str1, str2);
            strcat(str1, image_file_name);
            system(str1);
        }
        
private:
        void generate_dot_text()
        {
            for (auto& node : nodes_)
            {
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

            for (auto& link : links_)
            {
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