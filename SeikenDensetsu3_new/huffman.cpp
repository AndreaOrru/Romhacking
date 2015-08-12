#include <queue>
#include "bytepair.hpp"
#include "rom.hpp"
#include "huffman.hpp"


#include <iostream>
using namespace std;


namespace Huffman
{

struct Node
{
    u16 symbol;
    int weight;

    u64 bits = 0;
    int n_bits = 0;

    Node* parent = nullptr;
    Node* left   = nullptr;
    Node* right  = nullptr;

    inline bool is_leaf() const { return (!left and !right); }

    Node(u16 symbol, int weight) : symbol(symbol), weight(weight) {};
    Node(Node* l, Node* r) : left(l), right(r)
    {
        l->parent = r->parent = this;
        weight = l->weight + r->weight;
    }
    ~Node()
    {
        if (left)  delete left;
        if (right) delete right;
    }
};


vector<u16> encoded_tree;


inline u16 xba(u16 v)
{
    u8 l =  v & 0x00FF;
    u8 h = (v & 0xFF00) >> 8;

    return (l << 8) | h;
}

const vector<u16>& get_encoded_tree()
{
    return encoded_tree;
}

vector<u16> encode_tree(const Node* tree)
{
    vector<u16> data;
    queue<size_t> tags;
    queue<const Node*> q;

    q.push(tree);
    while (not q.empty())
    {
        const Node* n = q.front(); q.pop();

        if (n->is_leaf())
        {
            data[tags.front()] = 0x8000 | n->symbol;
            tags.pop();
        }
        else
        {
            if (not tags.empty())
            {
                data[tags.front()] = 0x1200 + 2*data.size();
                tags.pop();
            }

            tags.push(data.size());
            data.push_back(0x0000);
            tags.push(data.size());
            data.push_back(0x0000);

            q.push(n->left);
            q.push(n->right);
        }
    }
    return data;
}

vector<u16>* encode_data(const list<u16>& data, const vector<Node*>& leaves)
{
    auto* out = new vector<u16>;
    size_t b = 0;

    for (auto v: data)
    {
        u64 bits     = leaves[v]->bits;
        int to_write = leaves[v]->n_bits;

        while (to_write > 0)
        {
            out->resize(b/16 + 1, 0);

            int can_write = 16 - b%16;
            if (to_write <= can_write)
                out->at(b/16) |= (bits << (can_write - to_write));
            else
                out->at(b/16) |= (bits >> (to_write - can_write));
            b        += min(to_write, can_write);
            to_write -= min(to_write, can_write);
        }
    }

    for (size_t i = 0; i < out->size(); i++)
        out->at(i) = xba(out->at(i));
    return out;
}

vector<vector<u16>> compress(const vector<Block>& blocks)
{
    vector<Node*> leaves(0x580, nullptr);
    vector<int> occ(0x580, 0);

    vector<list<u16>> datas = BytePair::compress(blocks);

    for (auto& data: datas)
        for (auto symbol: data)
            occ[symbol]++;

    auto cmp = [](const Node* x, const Node* y) { return x->weight > y->weight; };
    priority_queue<Node*,vector<Node*>,decltype(cmp)> q(cmp);

    for (u16 symbol = 0; symbol < 0x580; symbol++)
        if (occ[symbol] > 0)
            q.push(leaves[symbol] = new Node(symbol, occ[symbol]));

    while (q.size() >= 2)
    {
        Node* n1 = q.top(); q.pop();
        Node* n2 = q.top(); q.pop();
        q.push(new Node(n1, n2));
    }
    Node* tree = q.top();

    for (auto n: leaves)
        if (n != nullptr)
            for (Node* p = n; p->parent != nullptr; p = p->parent)
                n->bits |= (p == p->parent->right) << n->n_bits++;

    vector<vector<u16>> encoded_datas;
    for (auto& data: datas)
        encoded_datas.push_back(*encode_data(data, leaves));

    encoded_tree = encode_tree(tree);
    delete tree;
    return encoded_datas;
}

vector<u8> decompress(const Block& block)
{
    vector<u8> data;

    u32 i = block.begin;
    u16 bits;
    int b = 1;

    while (i <= block.end)
    {
        u16 node = ROM::rd_w(0x3E6600);

        while (not (node & 0x8000))
        {
            b--;
            if (b == 0)
            {
                bits = xba(ROM::rd_w(i));
                i += 2;
                b = 16;
            }

            if (bits & 0x8000)
                node += 2;
            bits <<= 1;
            node = ROM::rd_w(0x3E6600 + node);
        }

        const vector<u8>& s = BytePair::get(node & 0x7FFF);
        data.insert(data.end(), s.begin(), s.end());
    }
    return data;
}

}
