#include <queue>
#include "bytepair.hpp"
#include "rom.hpp"
#include "huffman.hpp"

using namespace std;


namespace Huffman
{

struct Node
{
    u16 symbol;
    int weight;

    u64 bits   = 0;
    int n_bits = 0;

    Node* parent = nullptr;
    Node* left   = nullptr;
    Node* right  = nullptr;

    inline bool is_leaf() const { return (left == nullptr && right == nullptr); }

    Node(u16 symbol, int weight) : symbol(symbol), weight(weight) {};
    Node(Node* l, Node* r) : left(l), right(r)
    {
        l->parent = r->parent = this;
        weight = l->weight + r->weight;
    }
    ~Node()
    {
        if (left  != nullptr) delete left;
        if (right != nullptr) delete right;
    }
};


vector<u16> encoded_tree;


const vector<u16>& get_encoded_tree()
{
    return encoded_tree;
}

vector<u16> encode_tree(const Node* tree)
{
    vector<u16> data;
    queue<int> tags;
    queue<const Node*> q;

    q.push(tree);
    while (not q.empty())
    {
        const Node* n = q.front(); q.pop();

        if (n->is_leaf())
        {
            data[tags.front()] = (u16) (0x8000 | n->symbol);
            tags.pop();
        }
        else
        {
            if (not tags.empty())
            {
                data[tags.front()] = (u16) (0x1200 + 2*data.size());
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

vector<u8> encode_data(const list<u16>& data, const vector<Node*>& leaves)
{
    vector<u8> out;
    unsigned b = 0;

    for (auto v: data)
    {
        u64 bits     = leaves[v]->bits;
        int to_write = leaves[v]->n_bits;

        while (to_write > 0)
        {
            out.resize(b/8 + 1, 0);

            int can_write = 8 - b%8;
            if (to_write <= can_write)
                out[b/8] |= (bits << (can_write - to_write));
            else
                out[b/8] |= (bits >> (to_write - can_write));
            b        += min(to_write, can_write);
            to_write -= min(to_write, can_write);
        }
    }
    return out;
}

vector<vector<u8>> compress(const vector<Block>& blocks)
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

    vector<vector<u8>> encoded_datas;
    for (auto& data: datas)
        encoded_datas.push_back(encode_data(data, leaves));

    encoded_tree = encode_tree(tree);
    delete tree;

    return encoded_datas;
}

vector<u8> decompress(const Block& block)
{
    vector<u8> data;

    int i = block.begin;
    u8 bits = 0;
    int b = 1;

    while (i < block.end)
    {
        u16 node = ROM::rd_w(0x3E6600);

        while (not (node & 0x8000))
        {
            b--;
            if (b == 0)
            {
                bits = ROM::rd(i++);
                b = 8;
            }

            if (bits & 0x80)
                node += 2;
            bits <<= 1;
            node = ROM::rd_w(0x3E6600 + node);
        }

        const vector<u8>& s = BytePair::get((u16) (node & 0x7FFF));
        data.insert(data.end(), s.begin(), s.end());
    }
    return data;
}

}
