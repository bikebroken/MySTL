#ifndef RBTREE_H_
#define RBTREE_H_
#include "pair.h"
#include "RBtreeIterator.h"
#include "construct.h"
#include "allocator.h"
namespace MySTL{
	template<class Key, class Value, class KeyOfValue, class Compare,
			 class Alloc = alloc>
	class rb_tree{
	protected:
		typedef void* void_pointer;
		typedef __rb_tree_node_base* base_ptr;
		typedef __rb_tree_node<Value> rb_tree_node;
		typedef simple_alloc<rb_tree_node, Alloc> re_tree_node_allocator;
	public:
		typedef Key key_type;
		typedef Value value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef const value_type& const_reference;
		typedef rb_tree_node* link_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef __rb_tree_color_type color_type;
		typedef value_type& reference;
	protected:
		link_type get_node() { return rb_tree_node_allocator::allocate(); }
		void put_node(link_type p){ rb_tree_node_allocator::deallocate(p); }
		void destroy(Value* value_field) {}
		
		link_type create_node(const value_type& x){
			link_type tmp = get_node();	//配置空间
			__STL_TRY{
				construct(&tmp->value_field, x);	//构造内容
			__STL_UNWIND(put_node(tmp));
			return tmp;
		}

		link_type clone_node(link_type x){	//复制一个node
			link_type tmp = create_node(x->value_field);
			tmp->color = x->color;
			tmp->left = 0;
			tmp->right = 0;	
			return tmp;
		}

		void destroy_node(link_type p){
			destroy(&p->value_field);	//析构内容，将value_field 的地址传给该函数
			put_node(p);				//释放内容
		}
		
		void clear(){}
		
	protected:
		// RB-tree 只以三笔数据表现
		size_type node_count;	//追踪记录树的大小（节点数量）
		link_type header;		//实现的a trick,与root互为父节点，header的左右孩子都是root
		Compare key_compare;	//传入的节点值大小比较准则

		//以下三个函数用来方便取得header的成员
		link_type& root() const { return (link_type&) header->parent; }
		link_type& leftmost() const { return (link_type&) header->left; }
		link_type& rightmost() const { return (link_type&) header->right; }
			
		//以下六个函数用来方便取得节点 x 的成员
		static link_type& left(link_type x)
		  { return (link_type&)(x->left); }
		
		static link_type& right(link_type x)
		  { return (link_type&)(x->right); }
		
		static link_type& parent(link_type x)
		  { return (link_type&)(x->parent); }

		
		static reference& value(link_type x)
		  { return x->value_field; }
		
		static const Key& key(link_type x)
		  { return KeyOfValue() (x->parent); }

		static color_type& color(link_type x)
		  { return (color_type&)(x->color); }
		
	    //以下6个函数用来方便取得x的成员
		static link_type& left(base_ptr x)
		  { return (link_type&)(x->left);}
		
		static link_type& right(base_ptr x)
		  { return (link_type&)(x->right);}
		  
		static link_type& parent(base_ptr x) 
		  { return (link_type&)(x->parent);}

		static reference value(base_ptr x)
		  { return ((link_type)x)->value_field;}

		static const Key& key(base_ptr x)
		  { return KeyOfValue() (value(link_type(x)));}

		static color_type& color(base_ptr x)
	      { return (color_type&) (link_type(x)->color);}

		//求极大值和极小值，node class 有实现此功能的，交给他们完成即可
		static link_type minimum(link_type x){
			return (link_type) __rb_tree_node_base::minimum(x);
		}
		static link_type maximum(link_type x){
			return (link_type) __rb_tree_node_base::maximum(x);
		}
	
	public:
		typedef __rb_tree_iterator<value_type, reference, pointer> iterator;
	
	private:			//新值插入点            插入点的父节点    新值
		iterator __insert(base_ptr x_, base_ptr y_, const Value& v);
		
		link_type __copy(link_type x, link_type p);
		void __erase(link_type x);
		void init(){
			header = get_node();	//产生一个节点空间，令header指向它
			color(header) = __rb_tree_red;	//令header为红色，用来区分header和root
			root() = 0;
			leftmost() = header;	//令header的左子节点为自己
			rightmost() = header;	//令header的右子节点为自己
		}
	
	public:
		rb_tree(const Compare& comp = Compare())
			: node_count(0), key_compare(comp) { init(); }
	 	~rb_tree(){
			clear();
			put_node(header);
		}
		
		rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& 
			operator=(const rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& x);

	public:
		Compare key_comp() const { return key_compare; }
		iterator begin() { return leftmost(); }	//RB树的起头为最左（最小）节点处
		iterator end() { return header; }		//RB树的终点为header所指处
		bool empty() const { return node_count == 0; }
		size_type size() const { return node_count; }
		size_type max_size() const { return size_type(-1); }
	
	public:
		//将x插入RB-tree中，保持节点独一无二，如果插入失败返回的是指向该节点的迭代器
		pair<iterator, bool> insert_unique(const value_tpye& x);
		//将x插入到RB-tree中（允许节点值重复）
		iterator insert_equal(const value_type& x);
	};
	
	template<class Key, class Value, class KeyOfValue, class Compare, class Alloc>
	typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator
	rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::insert_equal(const Value& v)
	{
		link_type y =header;
		link_type x = root();	//从根节点开始
		while (x != 0){			//从跟节点开始， 往下寻找适当的插入点
			y = x;
			x = key_compare(KeyOfValue()(v), key(x)) ? left(x) : right(x);
			//以上，遇到“大”则往左，遇到“小于等于”则往右
		}
		return __insert(x, y, v);
		//以上，x为新值插入点，y为插入点之父节点，v为新值
	}
	
	//插入新值，节点键值不允许重复，若重复则插入无效
	//注意，返回值是个pair，第一个元素是个RB-tree迭代器，指向新增节点。
	//第二叉素表示插入成功与否
	template<class Key, class Value, class KeyOfValue, class Compare, class Alloc>
	pair<iterator, bool>insert_unique(const value_type& v){
		link_type y = header;
		link_type x = root();	
		bool comp = true;	
		while (x != 0){			//从根节点开始，往下寻找适当的插入点
			y = x;
			comp = key_compare(KeyOfValue()(v), key(x));	//v键值小于目前节点之键值？
			x = comp ? left(x) : right(x);
		}

		//离开while后，y所指插入点的父节点（此时的它必为叶节点）
		iterator j = iterator(y);
		if (comp)
			if(j == begin())
				return pair<iterator, bool>(__insert(x, y, v), true);
			else
				--j;
		if(key_compare(key(j.node), KeyOfValue()(v)))
			return pair<iterator, bool>(__insert(x, y, v), true);
	
		return pair<iterator, bool>(j, false);
	}
	
}
#endif

 






