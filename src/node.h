//
// Created by gpf on 2020/10/7.
//

#ifndef MYLEXER_NODE_H
#define MYLEXER_NODE_H


#include <vector>
#include <memory>



class CharClass;
class Edge;
typedef std::vector<Edge*> Edges;
class Node;

enum EDGEKIND{
    kNull,
    kCharClass,
    kStart,
    kEnd,
};

//nfa node
class Node{
    friend class NFA;
    friend class DFA;
private:
    Edges outedges_;
    int index_;
    bool isAccept_;
public:
    Node(int i):index_(i){}
    void AddEdge(Edge* e){outedges_.push_back(e);}
    void CopyEdges(Node* node){outedges_.assign(node->outedges_.begin(), node->outedges_.end());}
};

typedef std::unique_ptr<Node> NodePtr;
typedef std::pair<Node*,Node*> NodePair;

//nfa edge
class Edge{
    friend class NFA;
    friend class DFA;
private:
    int kind;
    CharClass* cclass= nullptr;
    Node* dst= nullptr;
public:
    Edge(){}
    Edge(Node* d,EDGEKIND ek = kNull):kind(ek),dst(d){}
};

//arena for dfa nodes and edges
class Arena {
public:
    Arena(){
        nodes.reserve(256);
        edges.reserve(256);
    }
    Node* newNode(int i){
        nodes.emplace_back(std::make_unique<Node>(i));
        return nodes.back().get();
    }
    Edge* newEdge(Node* d,EDGEKIND ek = kNull){
        edges.emplace_back(std::make_unique<Edge>(d,ek));
        return edges.back().get();
    }


private:
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<std::unique_ptr<Edge>> edges;
};

#endif //MYLEXER_NODE_H
