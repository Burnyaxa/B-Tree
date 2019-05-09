#include<iostream> 
#include<string>
#include <vector>
#include <ctime>
#include <fstream>
#include <sstream>
#include <queue>
#include <exception>
#include <stdio.h>
#include <Windows.h>
using namespace std;

int g_count = 0;

ifstream fin("DataBase.txt");
ofstream out;

struct elem
{
	int key;
	vector<int> vek;
	elem(int value = 0, int key = 0) : key(key) {
		vek.push_back(value);
	};
};

// A BTree node 
class BTreeNode
{

	elem* keys;  // An array of keys 
	int t;      // Minimum degree (defines the range for number of keys) 
	BTreeNode** C; // An array of child pointers 
	int n;     // Current number of keys 
	bool leaf; // Is true when node is leaf. Otherwise false 

public:

	BTreeNode(int _t, bool _leaf);   // Constructor 

	// A function to traverse all nodes in a subtree rooted with this node 
	void traverse(ofstream& rewrite);

	// A function to search a key in subtree rooted with this node. 
	BTreeNode* search(int k);   // returns NULL if k is not present. 

	void addValue(int key, int value);

	void delValue(int key, int value);

	// A function that returns the index of the first key that is greater 
	// or equal to k 
	int findKey(int k);

	// A utility function to insert a new key in the subtree rooted with 
	// this node. The assumption is, the node must be non-full when this 
	// function is called 
	void insertNonFull(int k, int value);

	// A utility function to split the child y of this node. i is index 
	// of y in child array C[].  The Child y must be full when this 
	// function is called 
	void splitChild(int i, BTreeNode* y);

	// A wrapper function to remove the key k in subtree rooted with 
	// this node. 
	void remove(int k);

	// A function to remove the key present in idx-th position in 
	// this node which is a leaf 
	void removeFromLeaf(int idx);

	// A function to remove the key present in idx-th position in 
	// this node which is a non-leaf node 
	void removeFromNonLeaf(int idx);

	// A function to get the predecessor of the key- where the key 
	// is present in the idx-th position in the node 
	int getPred(int idx);

	// A function to get the successor of the key- where the key 
	// is present in the idx-th position in the node 
	int getSucc(int idx);

	// A function to fill up the child node present in the idx-th 
	// position in the C[] array if that child has less than t-1 keys 
	void fill(int idx);

	// A function to borrow a key from the C[idx-1]-th node and place 
	// it in C[idx]th node 
	void borrowFromPrev(int idx);

	// A function to borrow a key from the C[idx+1]-th node and place it 
	// in C[idx]th node 
	void borrowFromNext(int idx);

	// A function to merge idx-th child of the node with (idx+1)th child of 
	// the node 
	void merge(int idx);


	void ShowTree();


	// Make BTree friend of this so that we can access private members of 
	// this class in BTree functions 
	friend class BTree;
	friend class BTreePrinter;
};

class BTree
{
	BTreeNode* root; // Pointer to root node 
	int t;  // Minimum degree 
	vector<elem> test;
	bool check(vector<elem>test, int k);

public:

	// Constructor (Initializes tree as empty) 
	BTree(int _t)
	{
		root = NULL;
		t = _t;
	}

	void traverse(ofstream& rewrite)
	{
		if (root != NULL) root->traverse(rewrite);
	}

	void ShowTree()
	{
		if (root != NULL) root->ShowTree();
	}
	// function to search a key in this tree 
	BTreeNode* search(int k)
	{
		return (root == NULL) ? NULL : root->search(k);
	}
	BTreeNode* getRoot() {
		return this->root;
	}

	// The main function that inserts a new key in this B-Tree 
	void insert(int k, int value);

	// The main function that removes a new key in thie B-Tree 
	void remove(int k);

	void addValue(int key, int value);

	void delValue(int key, int value);
	vector<int> searchFor(int k);

	void insertFor(int k, int value, int flag);
	void edit(int k, int value);
	vector<int> interpolationSearch(int k);
	friend class BTreePrinter;
};



BTreeNode::BTreeNode(int t1, bool leaf1)
{
	// nopy the given minimum degree and leaf property 
	t = t1;
	leaf = leaf1;

	// Allocate memory for maximum number of possible keys 
	// and child pointers 
	keys = new elem[2 * t - 1];
	C = new BTreeNode * [2 * t];

	// Initialize the number of keys as 0 
	n = 0;
}

// A utility function that returns the index of the first key that is 
// greater than or equal to k 
int BTreeNode::findKey(int k)
{
	int idx = 0;
	while (idx < n && keys[idx].key < k)
		++idx;
	return idx;
}

// A function to remove the key k from the sub-tree rooted with this node 
void BTreeNode::remove(int k)
{
	int idx = findKey(k);

	// The key to be removed is present in this node 
	if (idx < n && keys[idx].key == k)
	{

		// If the node is a leaf node - removeFromLeaf is called 
		// Otherwise, removeFromNonLeaf function is called 
		if (leaf)
			removeFromLeaf(idx);
		else
			removeFromNonLeaf(idx);
	}
	else
	{

		// If this node is a leaf node, then the key is not present in tree 
		if (leaf)
		{
			cout << "The key " << k << " is does not exist in the tree\n";
			return;
		}

		// The key to be removed is present in the sub-tree rooted with this node 
		// The flag indicates whether the key is present in the sub-tree rooted 
		// with the last child of this node 
		bool flag = ((idx == n) ? true : false);

		// If the child where the key is supposed to exist has less that t keys, 
		// we fill that child 
		if (C[idx]->n < t)
			fill(idx);

		// If the last child has been merged, it must have merged with the previous 
		// child and so we recurse on the (idx-1)th child. Else, we recurse on the 
		// (idx)th child which now has atleast t keys 
		if (flag && idx > n)
			C[idx - 1]->remove(k);
		else
			C[idx]->remove(k);
	}
	return;
}

// A function to remove the idx-th key from this node - which is a leaf node 
void BTreeNode::removeFromLeaf(int idx)
{

	// Move all the keys after the idx-th pos one place backward 
	for (int i = idx + 1; i < n; ++i)
		keys[i - 1] = keys[i];

	// Reduce the count of keys 
	n--;

	return;
}

// A function to remove the idx-th key from this node - which is a non-leaf node 
void BTreeNode::removeFromNonLeaf(int idx)
{

	int k = keys[idx].key;

	// If the child that precedes k (C[idx]) has atleast t keys, 
	// find the predecessor 'pred' of k in the subtree rooted at 
	// C[idx]. Replace k by pred. Recursively delete pred 
	// in C[idx] 
	if (C[idx]->n >= t)
	{
		int pred = getPred(idx);
		keys[idx] = pred;
		C[idx]->remove(pred);
	}

	// If the child C[idx] has less that t keys, examine C[idx+1]. 
	// If C[idx+1] has atleast t keys, find the successor 'succ' of k in 
	// the subtree rooted at C[idx+1] 
	// Replace k by succ 
	// Recursively delete succ in C[idx+1] 
	else if (C[idx + 1]->n >= t)
	{
		int succ = getSucc(idx);
		keys[idx] = succ;
		C[idx + 1]->remove(succ);
	}

	// If both C[idx] and C[idx+1] has less that t keys,merge k and all of C[idx+1] 
	// into C[idx] 
	// Now C[idx] contains 2t-1 keys 
	// Free C[idx+1] and recursively delete k from C[idx] 
	else
	{
		merge(idx);
		C[idx]->remove(k);
	}
	return;
}

// A function to get predecessor of keys[idx] 
int BTreeNode::getPred(int idx)
{
	// Keep moving to the right most node until we reach a leaf 
	BTreeNode* cur = C[idx];
	while (!cur->leaf)
		cur = cur->C[cur->n];

	// Return the last key of the leaf 
	return cur->keys[cur->n - 1].key;
}

int BTreeNode::getSucc(int idx)
{

	// Keep moving the left most node starting from C[idx+1] until we reach a leaf 
	BTreeNode* cur = C[idx + 1];
	while (!cur->leaf)
		cur = cur->C[0];

	// Return the first key of the leaf 
	return cur->keys[0].key;
}

// A function to fill child C[idx] which has less than t-1 keys 
void BTreeNode::fill(int idx)
{

	// If the previous child(C[idx-1]) has more than t-1 keys, borrow a key 
	// from that child 
	if (idx != 0 && C[idx - 1]->n >= t)
		borrowFromPrev(idx);

	// If the next child(C[idx+1]) has more than t-1 keys, borrow a key 
	// from that child 
	else if (idx != n && C[idx + 1]->n >= t)
		borrowFromNext(idx);

	// Merge C[idx] with its sibling 
	// If C[idx] is the last child, merge it with with its previous sibling 
	// Otherwise merge it with its next sibling 
	else
	{
		if (idx != n)
			merge(idx);
		else
			merge(idx - 1);
	}
	return;
}

// A function to borrow a key from C[idx-1] and insert it 
// into C[idx] 
void BTreeNode::borrowFromPrev(int idx)
{

	BTreeNode* child = C[idx];
	BTreeNode* sibling = C[idx - 1];

	// The last key from C[idx-1] goes up to the parent and key[idx-1] 
	// from parent is inserted as the first key in C[idx]. Thus, the  loses 
	// sibling one key and child gains one key 

	// Moving all key in C[idx] one step ahead 
	for (int i = child->n - 1; i >= 0; --i)
		child->keys[i + 1] = child->keys[i];

	// If C[idx] is not a leaf, move all its child pointers one step ahead 
	if (!child->leaf)
	{
		for (int i = child->n; i >= 0; --i)
			child->C[i + 1] = child->C[i];
	}

	// Setting child's first key equal to keys[idx-1] from the current node 
	child->keys[0] = keys[idx - 1];

	// Moving sibling's last child as C[idx]'s first child 
	if (!child->leaf)
		child->C[0] = sibling->C[sibling->n];

	// Moving the key from the sibling to the parent 
	// This reduces the number of keys in the sibling 
	keys[idx - 1] = sibling->keys[sibling->n - 1];

	child->n += 1;
	sibling->n -= 1;

	return;
}

// A function to borrow a key from the C[idx+1] and place 
// it in C[idx] 
void BTreeNode::borrowFromNext(int idx)
{

	BTreeNode* child = C[idx];
	BTreeNode* sibling = C[idx + 1];

	// keys[idx] is inserted as the last key in C[idx] 
	child->keys[(child->n)] = keys[idx];

	// Sibling's first child is inserted as the last child 
	// into C[idx] 
	if (!(child->leaf))
		child->C[(child->n) + 1] = sibling->C[0];

	//The first key from sibling is inserted into keys[idx] 
	keys[idx] = sibling->keys[0];

	// Moving all keys in sibling one step behind 
	for (int i = 1; i < sibling->n; ++i)
		sibling->keys[i - 1] = sibling->keys[i];

	// Moving the child pointers one step behind 
	if (!sibling->leaf)
	{
		for (int i = 1; i <= sibling->n; ++i)
			sibling->C[i - 1] = sibling->C[i];
	}

	// Increasing and decreasing the key count of C[idx] and C[idx+1] 
	// respectively 
	child->n += 1;
	sibling->n -= 1;

	return;
}

// A function to merge C[idx] with C[idx+1] 
// C[idx+1] is freed after merging 
void BTreeNode::merge(int idx)
{
	BTreeNode* child = C[idx];
	BTreeNode* sibling = C[idx + 1];

	// Pulling a key from the current node and inserting it into (t-1)th 
	// position of C[idx] 
	child->keys[t - 1] = keys[idx];

	// Copying the keys from C[idx+1] to C[idx] at the end 
	for (int i = 0; i < sibling->n; ++i)
		child->keys[i + t] = sibling->keys[i];

	// Copying the child pointers from C[idx+1] to C[idx] 
	if (!child->leaf)
	{
		for (int i = 0; i <= sibling->n; ++i)
			child->C[i + t] = sibling->C[i];
	}

	// Moving all keys after idx in the current node one step before - 
	// to fill the gap created by moving keys[idx] to C[idx] 
	for (int i = idx + 1; i < n; ++i)
		keys[i - 1] = keys[i];

	// Moving the child pointers after (idx+1) in the current node one 
	// step before 
	for (int i = idx + 2; i <= n; ++i)
		C[i - 1] = C[i];

	// Updating the key count of child and the current node 
	child->n += sibling->n + 1;
	n--;

	// Freeing the memory occupied by sibling 
	delete(sibling);
	return;
}

// The main function that inserts a new key in this B-Tree 
void BTree::insert(int k, int value)
{
	elem* o = new elem(value, k);
	// If tree is empty 
	if (root == NULL)
	{
		// Allocate memory for root 
		root = new BTreeNode(t, true);
		root->keys[0] = *o;  // Insert key 
		root->n = 1;  // Update number of keys in root 
	}
	else // If tree is not empty 
	{
		// If root is full, then tree grows in height 
		if (root->n == 2 * t - 1)
		{
			// Allocate memory for new root 
			BTreeNode* s = new BTreeNode(t, false);

			// Make old root as child of new root 
			s->C[0] = root;

			// Split the old root and move 1 key to the new root 
			s->splitChild(0, root);

			// New root has two children now.  Decide which of the 
			// two children is going to have new key 
			int i = 0;
			if (s->keys[0].key < k)
				i++;
			s->C[i]->insertNonFull(k, value);

			// Change root 
			root = s;
		}
		else  // If root is not full, call insertNonFull for root 
			root->insertNonFull(k, value);
	}
}

// A utility function to insert a new key in this node 
// The assumption is, the node must be non-full when this 
// function is called 
void BTreeNode::insertNonFull(int k, int value)
{
	elem* o = new elem(value, k);
	// Initialize index as index of rightmost element 
	int i = n - 1;

	// If this is a leaf node 
	if (leaf == true)
	{
		// The following loop does two things 
		// a) Finds the location of new key to be inserted 
		// b) Moves all greater keys to one place ahead 
		while (i >= 0 && keys[i].key > k)
		{
			keys[i + 1] = keys[i];
			i--;
		}

		// Insert the new key at found location 
		keys[i + 1] = *o;
		n = n + 1;
	}
	else // If this node is not leaf 
	{
		// Find the child which is going to have the new key 
		while (i >= 0 && keys[i].key > k)
			i--;

		// See if the found child is full 
		if (C[i + 1]->n == 2 * t - 1)
		{
			// If the child is full, then split it 
			splitChild(i + 1, C[i + 1]);

			// After split, the middle key of C[i] goes up and 
			// C[i] is splitted into two.  See which of the two 
			// is going to have the new key 
			if (keys[i + 1].key < k)
				i++;
		}
		C[i + 1]->insertNonFull(k, value);
	}
}

// A utility function to split the child y of this node 
// Note that y must be full when this function is called 
void BTreeNode::splitChild(int i, BTreeNode * y)
{
	// Create a new node which is going to store (t-1) keys 
	// of y 
	BTreeNode* z = new BTreeNode(y->t, y->leaf);
	z->n = t - 1;

	// Copy the last (t-1) keys of y to z 
	for (int j = 0; j < t - 1; j++)
		z->keys[j] = y->keys[j + t];

	// Copy the last t children of y to z 
	if (y->leaf == false)
	{
		for (int j = 0; j < t; j++)
			z->C[j] = y->C[j + t];
	}

	// Reduce the number of keys in y 
	y->n = t - 1;

	// Since this node is going to have a new child, 
	// create space of new child 
	for (int j = n; j >= i + 1; j--)
		C[j + 1] = C[j];

	// Link the new child to this node 
	C[i + 1] = z;

	// A key of y will move to this node. Find location of 
	// new key and move all greater keys one space ahead 
	for (int j = n - 1; j >= i; j--)
		keys[j + 1] = keys[j];

	// Copy the middle key of y to this node 
	keys[i] = y->keys[t - 1];

	// Increment count of keys in this node 
	n = n + 1;
}

// Function to traverse all nodes in a subtree rooted with this node 
void BTreeNode::traverse(ofstream & rewrite)
{
	string row;
	// There are n keys and n+1 children, travers through n keys 
	// and first n children 
	int i;
	for (i = 0; i < n; i++)
	{
		// If this is not leaf, then before printing key[i], 
		// traverse the subtree rooted with child C[i]. 
		if (leaf == false)
			C[i]->traverse(rewrite);
		rewrite << keys[i].key << ",";
		row += keys[i].key;
		row += ",";
		for (int j = 0; j < keys[i].vek.size(); j++) {
			rewrite << keys[i].vek[j] << ",";
			row += keys[i].vek[j];
			row += ",";
		}
		rewrite << "\n";
	}

	// Print the subtree rooted with last child 
	if (leaf == false)
		C[i]->traverse(rewrite);
}

void BTreeNode::ShowTree()
{
	// There are n keys and n+1 children, travers through n keys 
	// and first n children 
	int i;
	for (i = 0; i < n; i++)
	{
		// If this is not leaf, then before printing key[i], 
		// traverse the subtree rooted with child C[i]. 
		if (leaf == false)
			C[i]->ShowTree();
		cout << "|" << keys[i].key << ":";
		for (int j = 0; j < keys[i].vek.size(); j++) {
			cout << keys[i].vek[j] << " ";
		}
	}

	// Print the subtree rooted with last child 
	if (leaf == false)
		C[i]->ShowTree();
}

/*BTreeNode *BTreeNode::search(int value) {
	int k = (int)log(n);
	int i = pow(2,k);
	while (k >= 0 && keys[i] != value) {
		if (keys[i] > value) {
			i -= pow(2, --k);
		}
		else {
			i += pow(2, --k);
		}
	}
	if (keys[i] == value) return ;
	if (leaf == true) return nullptr;
	return  C[i]->search(k);
}*/

// Function to search key k in subtree rooted with this node 
BTreeNode* BTreeNode::search(int k)
{
	// Find the first key greater than or equal to k 
	int i = 0;
	while (i < n && k > keys[i].key) {
		g_count++;
		i++;
	}
	// If the found key is equal to k, return this node 
	if (keys[i].key == k) return this;

	// If key is not found here and this is a leaf node 
	if (leaf == true)
		return nullptr;

	// Go to the appropriate child 
	return C[i]->search(k);
}

void BTree::remove(int k)
{
	if (!root)
	{
		cout << "The tree is empty\n";
		return;
	}

	// Call the remove function for root 
	root->remove(k);

	// If the root node has 0 keys, make its first child as the new root 
	//  if it has a child, otherwise set root as NULL 
	if (root->n == 0)
	{
		BTreeNode* tmp = root;
		if (root->leaf)
			root = NULL;
		else
			root = root->C[0];

		// Free the old root 
		delete tmp;
	}
	return;
}

//void BTreeNode::addValue(int key, int value)
//{
//	int index = 0;
//	for (int i = 0; i < this->n; i++) {
//		index = i;
//		if (this->keys[i].key == key);
//		break;
//	}
//	this->keys[index].vek.push_back(value);
//}
//
//void BTreeNode::delValue(int key, int value)
//{
//	int indexK = 0;
//	
//	for (int i = 0; i < this->n; i++) {
//		indexK = i;
//		if (this->keys[indexK].key == key)
//		break;
//	}
//	for (int j = 0; j < this->keys->vek.size(); j++) {
//		if (this->keys[indexK].vek[j] == value) {
//			this->keys[indexK].vek.erase(this->keys[indexK].vek.begin() + j);
//		}
//	}
//}
void BTree::addValue(int key, int value)
{
	BTreeNode* q = search(key);
	elem* arr = q->keys;
	for (int i = 0; i < q->n; i++) {
		if (q->keys[i].key == key) {
			q->keys[i].vek.push_back(value);
			break;
		}
	}
}

void BTree::delValue(int key, int value)
{
	BTreeNode* q = search(key);
	elem* arr = q->keys;

	for (int i = 0; i < q->n; i++) {
		if (q->keys[i].key == key) {
			for (int j = 0; j < q->keys[i].vek.size(); j++) {
				if (q->keys[i].vek[j] == value) {
					q->keys[i].vek.erase(q->keys[i].vek.begin() + j);
					break;
				}
			}
			break;
		}
	}
}
vector<int> BTree::searchFor(int k) {
	BTreeNode* q = search(k);
	vector<int> empty;
	if (q == nullptr) return empty;
	int l;
	int w = (int)(log(q->n) + 1);
	int i = pow(2, w);
	if (q->keys[i].key < k && q->n>i) {
		l = (int)(log(q->n - i) + 1.5);
		i = q->n + l - pow(2, l);
	}
	while (w >= 0 && q->keys[i].key != k) {
		g_count++;
		if (q->keys[i].key > k) {
			i -= pow(2, --w);
		}
		else {
			i += pow(2, --w);
		}
	}
	if (q->keys[i].key == k) return q->keys[i].vek;
	/*for (int i = 0; i < q->n; i++) {
		if (q->keys[i].key == k) return q->keys[i].vek;
	}*/
	return empty;
}

bool BTree::check(vector<elem>test, int k) {
	if (search(k) != NULL) return true;
	return false;
}

void BTree::edit(int k, int value) {
	BTreeNode* q = search(k);
	//vector<elem> arr = q->keys;
	elem* arr = q->keys;
	for (int i = 0; i < q->n; i++) {
		if (q->keys[i].key == k) q->keys[i].vek.push_back(value);

	}
}

vector<int> BTree::interpolationSearch(int k)
{
	BTreeNode* q = search(k);
	vector<int> nullVector;
	int left = 0; // левая граница поиска (будем считать, что элементы массива нумеруются с нуля) 
	int right = q->n - 1;  // правая граница поиска 

	while (q->keys[left].key < k && k < q->keys[right].key) {
		int mid = (int)(left + (k - q->keys[left].key) * (right - left) / (q->keys[right].key - q->keys[left].key));  // индекс элемента, с которым будем проводить сравнение 
		if (q->keys[mid].key < k)
			left = mid + 1;
		else if (q->keys[mid].key > k)
			right = mid - 1;
		else
			return q->keys[mid].vek;
	}
	if (q->keys[left].key == k)
		return q->keys[left].vek;
	else if (q->keys[right].key == k)
		return q->keys[right].vek;
	else
		return nullVector;
}

void sort(vector<int> & search) {
	for (int i = 0; i < search.size(); i++) {
		for (int j = i; j < search.size(); j++) {
			if (search[j] < search[i]) {
				int temp = search[j];
				search[j] = search[i];
				search[i] = temp;
			}
		}
	}
}

int searchValue(vector<int> search, int value) {
	int l;
	int size = search.size();
	int w = (int)(log(size) + 0.5);
	int i = pow(2, w);
	if (search[i] < value && size>i) {
		l = (int)(log(size - i) + 1.5);
		i = size + l - pow(2, l);
	}
	while (w >= 0 && search[i] != value) {

		if (search[i] > value) {
			i -= pow(2, --w);
		}
		else {
			i += pow(2, --w);
		}
	}
	if (search[i] == value) return i;
}

void BTree::insertFor(int k, int value, int flag) {
	if (check(test, k)) {
		edit(k, value);
		return;
	}
	insert(k, value);
	return;
}


class BTreePrinter
{
	struct NodeInfo
	{
		std::string text;
		unsigned text_pos, text_end;  // half-open range
	};

	typedef std::vector<NodeInfo> LevelInfo;

	std::vector<LevelInfo> levels;

	//std::string node_text(elem const keys[], unsigned key_count);
	std::string node_text(elem const keys[], unsigned key_count);

	void before_traversal()
	{
		levels.resize(0);
		levels.reserve(10);   // far beyond anything that could usefully be printed
	}

	void visit(BTreeNode const* node, unsigned level = 0, unsigned child_index = 0);

	void after_traversal();

public:
	void print(BTree const& tree)
	{
		before_traversal();
		visit(tree.root);
		after_traversal();
	}
};

void BTreePrinter::visit(BTreeNode const* node, unsigned level, unsigned child_index)
{
	if (level >= levels.size())
		levels.resize(level + 1);

	LevelInfo & level_info = levels[level];
	NodeInfo info;

	info.text_pos = 0;
	if (!level_info.empty())  // one blank between nodes, one extra blank if left-most child
		info.text_pos = level_info.back().text_end + (child_index == 0 ? 2 : 1);

	info.text = node_text(node->keys, unsigned(node->n));

	if (node->leaf)
	{
		info.text_end = info.text_pos + unsigned(info.text.length());
	}
	else // non-leaf -> do all children so that .text_end for the right-most child becomes known
	{
		for (unsigned i = 0, e = unsigned(node->n); i <= e; ++i)  // one more pointer than there are keys
			visit(node->C[i], level + 1, i);

		info.text_end = levels[level + 1].back().text_end;
	}

	levels[level].push_back(info);
}

std::string BTreePrinter::node_text(elem const keys[], unsigned key_count)
{
	std::ostringstream os;
	char const* sep = "";

	os << "[";
	for (unsigned i = 0; i < key_count; ++i, sep = " ")
		os << sep << keys[i].key;
	os << "]";

	return os.str();
}

void print_blanks(unsigned n)
{
	while (n--)
		std::cout << ' ';
}

void BTreePrinter::after_traversal()
{
	for (std::size_t l = 0, level_count = levels.size(); ; )
	{
		auto const& level = levels[l];
		unsigned prev_end = 0;

		for (auto const& node : level)
		{
			unsigned total = node.text_end - node.text_pos;
			unsigned slack = total - unsigned(node.text.length());
			unsigned blanks_before = node.text_pos - prev_end;

			print_blanks(blanks_before + slack / 2);
			std::cout << node.text;

			if (&node == &level.back())
				break;

			print_blanks(slack - slack / 2);

			prev_end += blanks_before + total;
		}

		if (++l == level_count)
			break;

		std::cout << "\n\n";
	}

	std::cout << "\n";
}


// Driver program to test above functions 
void main()
{

	srand(time(0));
	BTree t(1000); 
	BTreePrinter printer;
	int n;
	int k;
	if (fin.is_open()) {
		cout << "Opening database..." << endl;
		string row;
		string temp;
		while (!fin.eof()) {
			getline(fin, row);
			if (row == "") continue;
			stringstream ss(row);
			getline(ss, row, ',');
			k = stoi(row);
			while (getline(ss, row, ',')) {
				n = stoi(row);
				t.insertFor(k, n, 0);
			}
		}
		//t.ShowTree();
	}
	else {
		int quanity;
		out.open("DataBase.txt", ios_base::trunc);
		cout << "Creating database..." << endl;
		cout << "How much random elements to enter?" << endl;
		cin >> quanity;
		for (int i = 0; i < quanity; i++) {
			k = i;
			n = rand() % 10000;
			t.insertFor(k, n, 1);
		}

	}
	char answer, editChoice;
	while (true) {
		int key, n1, n2;
		vector <int> values;
		cout << "What to do?" << endl;
		cout << "1. Add element" << endl;
		cout << "2. Delete element" << endl;
		cout << "3. Edit element" << endl;
		cout << "4. Find element" << endl;
		cout << "5. Show tree" << endl;
		cout << "6. Save & Exit" << endl;
		cin >> answer;
		switch (answer)
		{
		case '1':
			cout << "Key (must be uniqe) : "; cin >> key;
			cout << "Value : "; cin >> n1;
			t.insert(key, n1);
			cout << "Inserting pair : " << key << " " << n1 << endl;
			cout << "Your new database : " << endl;
			t.ShowTree();
			cout << endl << endl << endl << endl;
			cout << "Your new tree : " << endl;
			printer.print(t);
			cout << endl << endl << endl << endl;
			break;
		case '2':
			cout << "What element to delete?" << endl;
			cout << "Key : "; cin >> key;
			t.remove(key);
			cout << "Deleting key " << key << endl;
			cout << "Your new database : " << endl;
			t.ShowTree();
			cout << endl << endl << endl << endl;
			cout << "Your new tree : " << endl;
			printer.print(t);
			cout << endl << endl << endl << endl;
			break;
		case '3':
			cout << "What element to edit?" << endl;
			cout << "Key : "; cin >> key;
			cout << "Value : "; cin >> n1;
			cout << "What to do?" << endl;
			cout << "1. Add value" << endl;
			cout << "2. Delete value"; cin >> editChoice;
			switch (editChoice)
			{
			case '1':
				t.addValue(key, n1);
				cout << "Added " << n1 << " to " << key << endl;
				break;
			case '2':
				t.delValue(key, n1);
				cout << "Deleted " << n1 << " from " << key << endl;
				break;
			}
			cout << "Your new database : " << endl;
			t.ShowTree();
			cout << endl << endl << endl << endl;
			cout << "Your new tree : " << endl;
			printer.print(t);
			cout << endl << endl << endl << endl;
			break;
		case '4':
			cout << "What element to find?" << endl;
			cout << "Key : "; cin >> key;
			values = t.interpolationSearch(key);
			cout << "Founded values :";
			for (int i = 0; i < values.size(); i++) {
				cout << values[i] << " ";
			}
			cout << endl;
			break;
		case '6':
			cout << "Saving database..." << endl;
			t.traverse(out);
			fin.close();
			out.close();
			system("pause");
			return;
			break;
		case '5':
			cout << "Your database : " << endl;
			t.ShowTree();
			cout << endl << endl << endl << endl;
			cout << "Your tree : " << endl;
			printer.print(t);
			cout << endl << endl << endl << endl;
			break;
		}
	}
}