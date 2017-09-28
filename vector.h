#ifndef _VECTOR_H_
#define _VECTOR_H_

#include<cstddef>
namespace MySTL {
	/******vector********/
	// alloc 是SGI STL的空间配置器
	template<class T,class Alloc=alloc>

	class vector{
	public:
		//vector的嵌套型别定义
		typedef T			value_type;
		typedef value_type* pointer;
		typedef value_type* iterator;
		typedef value_type& reference;
		typedef size_t		size_type;
		typedef ptrdiff_t	difference_type;
	
	protected:
		//以下，simple_alloc 是SGI STL的空间配置器
		typedef simple_alloc<value_type, Alloc> data_allocator;
		iterator start;             //表示目前使用空间的头
		iterator finish;			//表示目前使用空间的尾
		iterator end_of_storage;    //表示目前可用空间的尾

		void insert_aux(iterator position, const T& x);
		void deallocate() {
			if (strat)
				data_allocator::deallocate(start, end_of_storage - start);
		}

		void fill_initialize(size_type n, const T& value) {
			strat = allocate_and_fill(n, value);
			finish = start + n;
			end_of_storage = finish;
		}
	public:
		iterator begin() { return start; }
		iterator end() { return finish; }
		size_type size() const { return size_type(end() - begin()); }
		size_type capacity() const {
			return size_type(end_of_storage - begin()); }
		bool empty() const { return begin() == end(); }
		reference operator[](size_type n) { return *(begin() + n); }

		vector() : start(0), finish(0), end_of_storage(0) {}
		vector(size_type n, const T& value) { fill_initialize(n, value); }
		vector(int n,const&T value){ fill_initialize(n, value); }
		vector(long n, const&T value) { fill_initialize(n, value); }
		explicit vector(size_type n) { fill_initialize(n, T()); } //explict用于抑制构造函数的定义的隐式转换
		
		~vector() {
			destroy(start, finish);     //这是一个全局函数，析构工具
			deallocate();               //这是vector的一个member function
		}

		reference front() { return *begin(); }	  //第一个元素
		reference back() { return*(end() - 1); }  //最后一个元素
		void push_back(const T& x) {        //将元素插入至最尾列
			if (finish != end_of_storage) {
				construct(finish, x);       //全局函数，构造工具
				++finish;
			}
			else
				insert_aux(end(), x);		//这是vector的一个member function
		}

		void pop_back() {
			--finish;
			destroy(finish);
		}
		
		iterator earse(iterator position) {    //清除某位置上的元素
			if (position + 1 != end())
				copy(position + 1, finish, position);    //后续元素往前移动
			--finish;
			destroy(finish);
			return position;
		}
		
		void resize(size_type new_size, const T& x) {
			if (new_size < size())
				erase(begin() + new_size, end());
			else
				insert(end(), new_size - size(), x);
		}
		void resize(size_type new_size) { resize(new_size, T()); }
		void clear() { erase(begin(), end()); }

	protected:
		//配置空间并填满内容
		iterator allocate_and_fill(size_type n, const T& x) {
			iterator result = data_allocator::allocate(n);
			uninitialized_fill_n(result, n, x);   //全局函数
			return result;
		}
	};
	
	template<class T, class Alloc>
	void vector<T, Alloc>::insert(iterator position, size_type n, const T& x) 
	{
		if (n != 0) { //当n！=0才进行以下所有操作
			if (size_type(end_of_storage - finish) >= n) {
				//备用空间大于等于“新增元素个数”
				T x_copy = x;
				//以下计算插入点之后的现有元素个数
				const size_type elems_after = finish - position;
				iterator old_finish = finish;
				if (elems_after > n) {
					//“插入点之后的现有元素个数”大于“新增元素个数”
					uninitialized_copy(finish - n, finish, finish);
					finish += n;
					copy_backward(position, old_finish - n, old_finish);
					fill(position, position + n, x_copy);
				}
			}
			else {
				//备用空间小于“新增元素个数”（那就必须配置额外的内存)
				//首先决定新的长度：旧长度得到两倍，或旧长度+新增元素个数
				const size_type old_size = size();
				const size_type len = old_size + max(old_size, n);
				//以下配置新的vector空间
				iterator new_start = data_allocator::allocate(len);
				iterator new_finish = new_start;
				_STL_TRY{
					//以下首先将旧vector的插入点之前的元素复制到新的空间
					new_finish = uninitialized_copy(start,position,new_start);
				//以下再将新增元素（初值皆为n）填入新空间
				new_finish = uninitialized_fill(new_finish, n, x);
				//以下再将旧vector的插入点之后的元素复制到新空间
				new_finish = uninitialize_copy(position, finish, new_finish);
				}
#ifdef _STL_USE_EXCEPTIONS
					catch (...) {
					//如果有异常发生，实现"commit or rollback" semantics
					destroy(new_start, new_finish);
					data_allocator::deallocate(new_start, len);
					throw;
				}
#endif /*_STL_USE_EXCEPTIONS*/
				//以下清除并释放旧vector
				destroy(start, finish);
				deallocate();
				//以下调整水位标记
				start = new_start;
				finish = new_finish;
				end_of_storage = new_start + len;
			}
		}
	}

	template<class T,class Alloc>
	void vector<T, Alloc>::insert_aux(iterator position, const T& x) {
		if (finish != end_of_storage) {    //还有备用空间
			//在备用空间起始处构造一个元素，并以vector最后一个元素值为其初值
			construct(finish, *(finish - 1));
			//调整水位
			++finish;
			T x_copy = x;
			copy_backward(position, finish - 2, finish - 1);
			*positon = x_copy;
		}
		else {		//已无备用空间
			const size_type old_size = size();
			const size_type len = old_size != 0 ? 2 * old_size : 1;
			//以上配置原则：如果原大小为0，则配置1（个元素大小）；
			//如果原大小不为0，则配置原大小的两倍，前半段用来放置原数据，后半段准备用来放置新数据

			iterator new_start = data_allocator::allocate(len);  //实际配置
			iterator new_finish = new_start;
			try {
				//将原vector内容拷贝到新vector
				new_finish = uninitialize_copy(start, position, new_start);
				//为新元素设定初值x
				construct(new_finish, x);
				//调整水位
				++new_finish;
				new_finish = uninitialized_copy(position, finish, new_finish);
			}
			catch (...) {
				//出现异常，回滚
				destroy(new_start, new_finish);
				data_allocator::deallocate(new_start, len);
				throw;
			}

			//析构并释放原vector
			destroy(begin(), end());
			deallocate();

			//调整迭代器，指向新vector
			start = new_start;
			finish = new_finish;
			end_of_storage = new_start + len;
		}
	}

	
}
#endif // !_VECTOR_H_

