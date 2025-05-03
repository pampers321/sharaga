/* узлы посещаются в порядке:

Сначала — правое поддерево

Потом — сам корень

Затем — левое поддерево
*/

#include <iostream>
#include <vector>
using namespace std;

// Узел двоичного дерева
struct Node {
    int Key;
    Node *Left, *Right;
    Node(int k) : Key(k), Left(nullptr), Right(nullptr) {}
};
typedef Node* PNode;

// Рекурсивно строит «сбалансированное» дерево из массива data[from…from+n-1]
PNode MakeTree(const vector<int>& data, int& from, int n) {
    if (n == 0) return nullptr;
    PNode root = new Node(data[from++]);
    int n1 = n / 2;
    int n2 = n - n1 - 1;
    root->Left  = MakeTree(data, from, n1);
    root->Right = MakeTree(data, from, n2);
    return root;
}

// Печать «горизонтального» дерева: reverse-inorder (right, root, left)
void PrintTree(PNode q, int indent = 0) {
    if (!q) return;
    PrintTree(q->Right, indent + 5);
    cout << string(indent, ' ') << q->Key << "\n";
    PrintTree(q->Left,  indent + 5);
}

int main() {
    int n;
    cout << "Сколько узлов добавить? ";
    if (!(cin >> n) || n < 0) {
        cerr << "Неверное количество\n";
        return 1;
    }

    vector<int> data;
    data.reserve(n);
    cout << "Введите " << n << " ключей через пробел или перевод строки:\n";
    for (int i = 0; i < n; ++i) {
        int x;
        cin >> x;
        data.push_back(x);
    }

    int pos = 0;
    PNode tree = MakeTree(data, pos, n);

    cout << "\nДерево (reverse inorder — right, root, left):\n\n";
    PrintTree(tree);

    // Освобождаем память
    // (простой способ — рекурсивно удалять узлы)
    // здесь для краткости пропущен, но в реальном коде нужно пройтись по всем узлам и delete

    return 0;
}
