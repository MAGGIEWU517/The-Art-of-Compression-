
/**
 *
 * twoDtree (pa3)
 * slight modification of a Kd tree of dimension 2.
 * twoDtree.cpp
 * This file will be used for grading.
 *
 */

#include "twoDtree.h"
#include "cs221util/PNG.h"
#include "cs221util/RGBAPixel.h"

#include <iostream>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;
using namespace cs221util;

/* given */
twoDtree::Node::Node(pair<int,int> ul, pair<int,int> lr, RGBAPixel a)
	:upLeft(ul),lowRight(lr),avg(a),left(NULL),right(NULL)
	{}

/* given */
twoDtree::~twoDtree(){
	clear();
}

/* given */
twoDtree::twoDtree(const twoDtree & other) {
	copy(other);
}

/* given */
twoDtree & twoDtree::operator=(const twoDtree & rhs){
	if (this != &rhs) {
		clear();
		copy(rhs);
	}
	return *this;
}


//==================================================================
 /**
    * Constructor that builds a twoDtree out of the given PNG.
    * Every leaf in the tree corresponds to a pixel in the PNG.
    * Every non-leaf node corresponds to a rectangle of pixels 
    * in the original PNG, represented by an (x,y) pair for the 
    * upper left corner of the rectangle and an (x,y) pair for 
    * lower right corner of the rectangle. In addition, the Node
    * stores a pixel representing the average color over the 
    * rectangle. 
    *
    * Every node's left and right children correspond to a partition
    * of the node's rectangle into two smaller rectangles. The node's
    * rectangle is split by the horizontal or vertical line that 
    * results in the two smaller rectangles whose sum of squared 
    * differences from their mean is as small as possible.
    *
    * The left child of the node will contain the upper left corner
    * of the node's rectangle, and the right child will contain the
    * lower right corner.
    *
   * This function will build the stats object used to score the 
   * splitting lines. It will also call helper function buildTree.
    */ 
twoDtree::twoDtree(PNG & imIn){
	stats s = stats(imIn);
	width = imIn.width();
	height = imIn.height();
	pair<int,int> ul = make_pair(0,0);
	pair<int,int> lr = make_pair(width-1, height-1);
	root = buildTree(s, ul,lr);
	
}

twoDtree::Node * twoDtree::buildTree(stats & s, pair<int,int> ul, pair<int,int> lr) {
	RGBAPixel a = s.getAvg(ul,lr);
	Node* node = new Node(ul, lr, a);
	int x1=ul.first;
	int x2=lr.first;
	int y1=ul.second;
	int y2=lr.second;

	if(x1==x2 && y1==y2){
		return node;
	}

	pair<int, int> sub1ul = ul;
	pair<int, int> sub2lr = lr;

	long smallest = 300000000000000;
	pair<int, int> smsub1lr;
	pair<int, int> smsub2ul;

	//cout<<"smallest begin: "<< smallest<<endl;
	//cout<<"smsub1lr begin: "<< smsub1lr.first<<endl;
	//cout<<"smsub2ul begin: "<< smsub2ul.first<<endl;


	for(int i=x1; i< x2; i++){
		pair<int, int> sub1lr= make_pair(i,y2);
		pair<int, int> sub2ul = make_pair(i+1, y1);

		long double sub1 = s.getScore(sub1ul,sub1lr);
		long double sub2 = s.getScore(sub2ul,sub2lr);

		if(sub1 + sub2 < smallest){
			smallest = sub1+sub2;
			smsub1lr = sub1lr;
			smsub2ul = sub2ul;
		}

	}

	for(int i=y1; i< y2; i++){
		pair<int, int> sub1lr= make_pair(x2,i);
		pair<int, int> sub2ul = make_pair(x1, i+1);

		long double sub1 = s.getScore(sub1ul,sub1lr);
		long double sub2 = s.getScore(sub2ul,sub2lr);

		if(sub1 + sub2 < smallest){
			smallest = sub1+sub2;
			smsub1lr = sub1lr;
			smsub2ul = sub2ul;
		}
	}


	node->left = buildTree(s, ul, smsub1lr);
	node->right = buildTree(s, smsub2ul, lr);
	return node;
}

/**
 * Render returns a PNG image consisting of the pixels
 * stored in the tree. may be used on pruned trees. Draws
 * every leaf node's rectangle onto a PNG canvas using the 
 * average color stored in the node.
*/

PNG twoDtree::render(){
	PNG rendpng = PNG(width, height);
	render(rendpng, root);

	return rendpng;
}

void twoDtree::render(PNG & rendpng, Node* subRoot){
	if(subRoot == NULL){
		return;
	}  
	//render this node and it's children
	int x1 = subRoot->upLeft.first;
	int y1 = subRoot->upLeft.second;
	int x2 = subRoot->lowRight.first;
	int y2 = subRoot->lowRight.second;

	for(int i = x1; i<= x2; i++){
		for(int j = y1; j <= y2; j++){
			*rendpng.getPixel(i,j)= subRoot->avg;
		}
	}
	render(rendpng, subRoot->left);
	render(rendpng, subRoot->right);
}

  /*
    *  Prune function trims subtrees as high as possible in the tree.
    *  A subtree is pruned (cleared) if at least pct of its leaves are within 
    *  tol of the average color stored in the root of the subtree. 
    *  Pruning criteria should be evaluated on the original tree, not 
    *  on a pruned subtree. (we only expect that trees would be pruned once.)
    *  
   * You may want a recursive helper function for this one.
    */
void twoDtree::prune(double pct, int tol){
	prune(pct, tol, root);	
}

void twoDtree::prune(double pct, int tol, Node* subRoot){
	if(subRoot == NULL){
		return;
	}
	if(subRoot->left == NULL && subRoot->right == NULL){
		return;
	}
	RGBAPixel subavg = subRoot->avg;
	long double tleaf = tolleaf(tol, subRoot, subavg);
	long double nleaf = leaf(subRoot);
	
	double ratio = (double)tleaf/(double)nleaf;

	

	if(ratio >= pct){
		clear(subRoot->left);
		subRoot->left =NULL;
		clear(subRoot->right);
		subRoot->right =NULL;

	}else{
		prune(pct, tol, subRoot->left);
		prune(pct, tol, subRoot->right);
	}

}


long twoDtree::tolleaf(int tol, Node* subRoot,RGBAPixel subavg){
	
	int r = subRoot->avg.r;
	int g = subRoot->avg.g;
	int b = subRoot->avg.b; 

	if(subRoot == NULL){
		return 0;
	}
	if(subRoot->left == NULL && subRoot->right == NULL){
		long double diff = (r-subavg.r)*(r-subavg.r)+(g-subavg.g)*(g-subavg.g)+(b-subavg.b)*(b-subavg.b);
		if(diff <= tol){
			return 1;
		}else{
			return 0;
		}
	}
	return tolleaf(tol, subRoot->left, subavg) + tolleaf(tol, subRoot->right, subavg);
}
//number leaves of the node
long twoDtree::leaf(Node* subRoot){

  if(subRoot == NULL){
		return 0;
	}
	if(subRoot->left == NULL && subRoot->right == NULL){
		return 1;
	}
	
	return leaf(subRoot->left) + leaf(subRoot->right);
}


void twoDtree::clear() {
    clear(root);
    root = NULL;
    height = 0;
    width = 0;
}

void twoDtree::clear(Node* subRoot)
{
	
    if (subRoot == NULL){
	    return;
	}

    clear(subRoot->left);
    clear(subRoot->right);

    subRoot = NULL;
    delete subRoot;

}


void twoDtree::copy(const twoDtree & orig){
	root = copynode(orig.root);
	height = orig.height;
	width = orig.width;
}

twoDtree::Node * twoDtree::copynode(Node* subRoot)
{
    if (subRoot == NULL){
	    return NULL;
	}

    // Copy this node and it's children
    Node* newNode = new Node(subRoot->upLeft, subRoot->lowRight, subRoot->avg);
    newNode->left = copynode(subRoot->left);
    newNode->right = copynode(subRoot->right);
    return newNode;
}



