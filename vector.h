#ifndef _VECTOR_H_
#define _VECTOR_H_

#include<cstddef>
namespace MySTL {
	/******vector********/
	// alloc ��SGI STL�Ŀռ�������
	template<class T,class Alloc=alloc>

	class vector{
	public:
		//vector��Ƕ���ͱ���
		typedef T			value_type;
		typedef value_type* pointer;
		typedef value_type* iterator;
		typedef value_type& reference;
		typedef size_t		size_type;
		typedef ptrdiff_t	difference_type;
	
	protected:
		//���£�simple_alloc ��SGI STL�Ŀռ�������
		typedef simple_alloc<value_type, Alloc> data_allocator;
		iterator start;             //��ʾĿǰʹ�ÿռ��ͷ
		iterator finish;			//��ʾĿǰʹ�ÿռ��β
		iterator end_of_storage;    //��ʾĿǰ���ÿռ��β

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
		explicit vector(size_type n) { fill_initialize(n, T()); } //explict�������ƹ��캯���Ķ������ʽת��
		
		~vector() {
			destroy(start, finish);     //����һ��ȫ�ֺ�������������
			deallocate();               //����vector��һ��member function
		}

		reference front() { return *begin(); }	  //��һ��Ԫ��
		reference back() { return*(end() - 1); }  //���һ��Ԫ��
		void push_back(const T& x) {        //��Ԫ�ز�������β��
			if (finish != end_of_storage) {
				construct(finish, x);       //ȫ�ֺ��������칤��
				++finish;
			}
			else
				insert_aux(end(), x);		//����vector��һ��member function
		}

		void pop_back() {
			--finish;
			destroy(finish);
		}
		
		iterator earse(iterator position) {    //���ĳλ���ϵ�Ԫ��
			if (position + 1 != end())
				copy(position + 1, finish, position);    //����Ԫ����ǰ�ƶ�
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
		//���ÿռ䲢��������
		iterator allocate_and_fill(size_type n, const T& x) {
			iterator result = data_allocator::allocate(n);
			uninitialized_fill_n(result, n, x);   //ȫ�ֺ���
			return result;
		}
	};
	
	template<class T, class Alloc>
	void vector<T, Alloc>::insert(iterator position, size_type n, const T& x) 
	{
		if (n != 0) { //��n��=0�Ž����������в���
			if (size_type(end_of_storage - finish) >= n) {
				//���ÿռ���ڵ��ڡ�����Ԫ�ظ�����
				T x_copy = x;
				//���¼�������֮�������Ԫ�ظ���
				const size_type elems_after = finish - position;
				iterator old_finish = finish;
				if (elems_after > n) {
					//�������֮�������Ԫ�ظ��������ڡ�����Ԫ�ظ�����
					uninitialized_copy(finish - n, finish, finish);
					finish += n;
					copy_backward(position, old_finish - n, old_finish);
					fill(position, position + n, x_copy);
				}
			}
			else {
				//���ÿռ�С�ڡ�����Ԫ�ظ��������Ǿͱ������ö�����ڴ�)
				//���Ⱦ����µĳ��ȣ��ɳ��ȵõ���������ɳ���+����Ԫ�ظ���
				const size_type old_size = size();
				const size_type len = old_size + max(old_size, n);
				//���������µ�vector�ռ�
				iterator new_start = data_allocator::allocate(len);
				iterator new_finish = new_start;
				_STL_TRY{
					//�������Ƚ���vector�Ĳ����֮ǰ��Ԫ�ظ��Ƶ��µĿռ�
					new_finish = uninitialized_copy(start,position,new_start);
				//�����ٽ�����Ԫ�أ���ֵ��Ϊn�������¿ռ�
				new_finish = uninitialized_fill(new_finish, n, x);
				//�����ٽ���vector�Ĳ����֮���Ԫ�ظ��Ƶ��¿ռ�
				new_finish = uninitialize_copy(position, finish, new_finish);
				}
#ifdef _STL_USE_EXCEPTIONS
					catch (...) {
					//������쳣������ʵ��"commit or rollback" semantics
					destroy(new_start, new_finish);
					data_allocator::deallocate(new_start, len);
					throw;
				}
#endif /*_STL_USE_EXCEPTIONS*/
				//����������ͷž�vector
				destroy(start, finish);
				deallocate();
				//���µ���ˮλ���
				start = new_start;
				finish = new_finish;
				end_of_storage = new_start + len;
			}
		}
	}

	template<class T,class Alloc>
	void vector<T, Alloc>::insert_aux(iterator position, const T& x) {
		if (finish != end_of_storage) {    //���б��ÿռ�
			//�ڱ��ÿռ���ʼ������һ��Ԫ�أ�����vector���һ��Ԫ��ֵΪ���ֵ
			construct(finish, *(finish - 1));
			//����ˮλ
			++finish;
			T x_copy = x;
			copy_backward(position, finish - 2, finish - 1);
			*positon = x_copy;
		}
		else {		//���ޱ��ÿռ�
			const size_type old_size = size();
			const size_type len = old_size != 0 ? 2 * old_size : 1;
			//��������ԭ�����ԭ��СΪ0��������1����Ԫ�ش�С����
			//���ԭ��С��Ϊ0��������ԭ��С��������ǰ�����������ԭ���ݣ�����׼����������������

			iterator new_start = data_allocator::allocate(len);  //ʵ������
			iterator new_finish = new_start;
			try {
				//��ԭvector���ݿ�������vector
				new_finish = uninitialize_copy(start, position, new_start);
				//Ϊ��Ԫ���趨��ֵx
				construct(new_finish, x);
				//����ˮλ
				++new_finish;
				new_finish = uninitialized_copy(position, finish, new_finish);
			}
			catch (...) {
				//�����쳣���ع�
				destroy(new_start, new_finish);
				data_allocator::deallocate(new_start, len);
				throw;
			}

			//�������ͷ�ԭvector
			destroy(begin(), end());
			deallocate();

			//������������ָ����vector
			start = new_start;
			finish = new_finish;
			end_of_storage = new_start + len;
		}
	}

	
}
#endif // !_VECTOR_H_

