// Author: Dhroov Pandey
// Last modified: November 25, 2026

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
    // Removes the edge from this node to `n`, and its label, if there is one
    void remove_neighbor(Node* n){
        for (size_t k = 0; k<this->_neighbors.size(); k++){
            if (this->_neighbors[k] != n) continue;
            this->_neighbors.erase(this->_neighbors.begin() + k);
            this->_edge_labels.erase(this->_edge_labels.begin() + k);
            return;   // add_edge never draws the same edge twice
        }
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
    // Removes every edge that points at the node at position `to` in nodes()
    void remove_edges_to(int to){
        Node* target = &this->_nodes[to];
        for (Node& n : this->_nodes)
            n.remove_neighbor(target);
    }
    private:
    std::vector<Node> _nodes;

};
