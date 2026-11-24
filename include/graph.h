#include <vector>

class Graph{
    public:
    Graph(){}
    const vector<Node>& nodes(){return this->_nodes;}
    void add_nodes(Node& n){this->_nodes.push_back(n);}
    private:
    vector<Node> _nodes;
    
};

class Node{
    public:
    Node(int i): _id(i){}
    void add_neighbor(Node* n){this->_neighbors.push_back(n);}
    int id(){return this->_id;}
    const std::vector<Node*> neighbors(){return this->_neighbors;}
    private:
    int _id;
    std::vector<Node*> _neighbors;
};