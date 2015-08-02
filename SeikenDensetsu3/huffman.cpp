#include <queue>
#include "bytepair.hpp"
#include "huffman.hpp"

using namespace std;

namespace Huffman
{

struct Node
{
    int sym = 0;
    int weight = 0;

    uint64_t bits = 0;
    int nBits = 0;

    Node* parent = 0;
    Node* left   = 0;
    Node* right  = 0;

    Node(int sym, int weight) : sym(sym), weight(weight) {};
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


uint16_t xba(uint16_t v)
{
    uint8_t l =  v & 0x00FF;
    uint8_t h = (v & 0xFF00) >> 8;

    return (l << 8) | h;
}

vector<uint16_t>* encode_tree(Node* tree)
{
    auto* encoding = new vector<uint16_t>;

    queue<Node*> q;
    queue<int> tags;

    q.push(tree);
    while (not q.empty())
    {
        Node* n = q.front(); q.pop();

        if (!n->left and !n->right)
        {
            encoding->at(tags.front()) = 0x8000 | n->sym;
            tags.pop();
        }
        else
        {
            if (not tags.empty())
            {
                encoding->at(tags.front()) = encoding->size()*2 + 0x1200;
                tags.pop();
            }
            if (n->left != 0)
            {
                tags.push(encoding->size());
                encoding->push_back(0x0000);
            }
            if (n->right != 0)
            {
                tags.push(encoding->size());
                encoding->push_back(0x0000);
            }
        }
        if (n->left)
            q.push(n->left);
        if (n->right)
            q.push(n->right);
    }

    return encoding;
}

void encode_data(vector<vector<uint16_t>*>& blocksData, vector<Node*>& nodes)
{
    for (int i = 0; i < blocksData.size(); i++)
    {
        //blocksData[i]->push_back(0x100);
        auto* compr = new vector<uint16_t>;
        int b = 0;

        for (auto sym: *blocksData[i])
        {
            uint64_t bits = nodes[sym]->bits;
            int   toWrite = nodes[sym]->nBits;

            while (toWrite > 0)
            {
                compr->resize(b/16 + 1, 0);

                int canWrite = 16 - b%16;
                if (toWrite <= canWrite)
                    compr->at(b/16) |= (bits << (canWrite - toWrite));
                else
                    compr->at(b/16) |= (bits >> (toWrite - canWrite));
                b += min(toWrite, canWrite);
                toWrite -= min(toWrite, canWrite);
            }
        }

        delete blocksData[i];
        for (auto& v: *compr)
            v = xba(v);
        blocksData[i] = compr;
    }
}

vector<uint16_t>* compress(vector<vector<uint16_t>*>& blocksData)
{
    vector<int> occ(0x0580, 0);
    vector<Node*> nodes(0x0580, 0);

    for (auto* block: blocksData)
        for (auto sym: *block)
            occ[sym]++;

    auto cmp = [](const Node* x, const Node* y) { return x->weight > y->weight; };
    priority_queue<Node*,vector<Node*>,decltype(cmp)> q(cmp);

    for (int sym = 0; sym < 0x0580; sym++)
        if (occ[sym] != 0)
        {
            auto* n = new Node(sym, occ[sym]);
            nodes[sym] = n;
            q.push(n);
        }

    while (q.size() >= 2)
    {
        Node* n1 = q.top(); q.pop();
        Node* n2 = q.top(); q.pop();
        q.push(new Node(n1, n2));
    }
    Node* tree = q.top();

    for (auto n: nodes)
        if (n)
        {
            Node* p = n;
            while (p->parent)
            {
                n->bits |= (p == p->parent->right) << n->nBits;
                n->nBits++;
                p = p->parent;
            }
        }

    auto* encodedTree = encode_tree(tree);
    encode_data(blocksData, nodes);
    delete tree;

    return encodedTree;
}

vector<uint8_t>* decompress(const uint16_t* comprData, uint32_t comprSize, const uint16_t* meta)
{
    auto* data = new vector<uint8_t>;

    uint32_t comprPtr = 0;
    uint16_t bits;
    uint8_t i = 1;

    while (comprPtr <= comprSize)
    {
        uint16_t metaPtr = meta[0];

        while (not (metaPtr & 0x8000))
        {
            if (--i == 0)
            {
                bits = xba(comprData[comprPtr / 2]);
                comprPtr += 2;
                i = 16;
            }

            bool b = bits & 0x8000;
            bits <<= 1;

            if (b)
                metaPtr += 2;

            metaPtr = meta[metaPtr / 2];
        }

        auto s = BytePair::decompress(metaPtr & 0x7FFF, meta);
        data->insert(data->end(), s.begin(), s.end());
    }

    //vector<uint8_t> end = { 0xFF, 0xFF };
    //auto occ = find_end(data->begin(), data->end(), end.begin(), end.end());
    //if (occ != data->end())
    //    data->resize(distance(data->begin(), occ) + end.size());

    return data;
}

}
