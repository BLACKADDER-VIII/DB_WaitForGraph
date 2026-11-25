#pragma once
#include <string>
#include <vector>

class Node{
    public:
    Node(int i): _id(i){}
    // Adds an edge from this node to `n`, carrying `label` as its attribute
    void add_neighbor(Node* n, const std::string& label = ""){
        this->_neighbors.push_back(n);
        this->_edge_labels.push_back(label);
    }
    int id() const {return this->_id;}
    const std::vector<Node*> neighbors() const {return this->_neighbors;}
    // edge_labels()[k] is the label of the edge to neighbors()[k]
    const std::vector<std::string>& edge_labels() const {return this->_edge_labels;}
    private:
    int _id;
    std::vector<Node*> _neighbors;
    std::vector<std::string> _edge_labels;   // Parallel to _neighbors
};

class Graph{
    public:
    Graph(){}
    const std::vector<Node>& nodes(){return this->_nodes;}
    void add_nodes(Node& n){this->_nodes.push_back(n);}
    // Draws a directed edge from the node at position `from` in nodes() to the
    // node at position `to`, carrying `label` as its attribute. Does nothing
    // if that edge already exists (the first label is kept).
    // Edges are pointers into _nodes, so add every node before the first edge:
    // a later add_nodes() may move the nodes and leave the edges dangling.
    void add_edge(int from, int to, const std::string& label = ""){
        Node* target = &this->_nodes[to];
        for (Node* n : this->_nodes[from].neighbors())
            if (n == target) return;
        this->_nodes[from].add_neighbor(target, label);
    }
    private:
    std::vector<Node> _nodes;

};
