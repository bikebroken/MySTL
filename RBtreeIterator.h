#ifndef _RBTREE_H_
#define _RBTREE_H_

#include "iterator.h"

namespace MySTL{
	typedef bool __rb_tree_color_type;
	const __rb_tree_color_type __rb_tree_red = false;	//红色为0
	const __rb_tree_color_type __rb_tree_black = true;	//黑色为1
	struct __rb_tree_node_base
	{
		typedef __rb_tree_color_type color_type;
		typedef __rb_tree_node_base* base_ptr;
		color_type color;
		base_ptr parent;
		base_ptr left;
		base_ptr right;
	
		static base_ptr minimum(base_ptr x)
		{
			while (x->left != 0) x = x->left;
			return x;
		}
	
		static base_ptr maximun(base_ptr x)
		{
			while (x->right != 0) x = x->right;
			return x;
		}
	};

	template <class Value>
	struct __rb_tree_node : public __rb_tree_node_base
	{
		typedef __rb_tree_node<Value>* link_type;
		Value value_field;	//节点值
	};

	//基层迭代器
	struct __rb_tree_base_iterator
	{
		typedef __rb_tree_node_base::base_ptr base_ptr;
		typedef bidirectional_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		base_ptr node; //它用来与容器之间产生一个连结关系（make a reference)
	
		//以下其实可实现operator++内，因为无他处会调用此函数了
		void increment()	
		{
			if(node->right != 0){			//如果有右子节点 状况（1）
				node = node->right;			//就向右走
				while(node->left != 0)		//然后一直往左子树走到底
					node = node->left;		//即是解答	
			}
			else{							//没有右子节点。状况（2）			
				base_ptr y = node->parent;	//找出父节点
				while(node == y->right ){	//如果现行节点本身是右子节点，	
					node = y;				//就一直上溯，直到“不为右子节点”止
					y = y->parent;
				}
				if(node->right != y)		//若此时的右子节点不等于此时的父节点
					node = y;				//状况（3）此时的父节点即为解答
			}								//否则此时的node为解答，状况（4）
		}

		void decrement(){
			if (node->color == __rb_tree_red &&
					node->parent->patent == node)
					node = node ->right;
			else if(node -> left != 0){	
				base_ptr y = node->left;
				while(y->right != 0)
					y = y->right;
				node = y;
			}
			else{
				base_ptr y = node->parent;
				while(node == y->left){
					node = y;
					y = y->parent;
				}
				node = y;
			}
		}		
	};	

	//RB-tree 的正规迭代器
	template<class Value, class Ref, class Ptr>
	struct __rb_tree_iterator:public __rb_tree_base_iterator
	{
		typedef Value value_tpye;
		typedef Ref reference;
		typedef Ptr pointer;
		typedef __rb_tree_iterator<Value, Value&, Value*> iterator;
    	typedef __rb_tree_iterator<Value, const Value&, const Value*> const_iterator;
    	typedef __rb_tree_iterator<Value, Ref, Ptr> self;
		typedef __rb_tree_node<Value>* link_type;
		
		__rb_tree_iterator(){}
		__rb_tree_iterator(link_type x){ node = x;}
		__rb_tree_iterator(const iterator& it){ node = it.node;}
		
		reference operator*() const { return link_type(node)->value_field; }
	  #ifndef __SGI_STL_NO_ARROW_OPERATOR
		pointer operator->() const { return &(operator*() ); }
	  #endif /* __SGI_STL_NO_ARROW_OPERATOR */
	
		self& operator++() { increment(); return *this; }
		self operator++(int){
			self tmp = *this;
			increment();
			reutrn tmp;
		}
		
		self& operator--() { decrement(); return *this; }
		self operator--(int){
			self tmp = *this;
			decrement();
			reutrn tmp;
		}

		bool operator==(const self& iter) const { return node == iter.node;}
    	bool operator!=(const self& iter) const { return node != iter.node;}

    	// bool operator==(const self& x) const {return node == x.node;}
		// bool operator!=(const self& x) const {return node != x.node;}
	};
}
		
#endif	






















