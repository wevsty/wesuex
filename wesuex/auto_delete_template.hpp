#pragma once
#include <vector>
using std::vector;
template <class T>
class AUTO_DELETE_TEMPLATE
{
public:
	vector<T> vec_delete_list;
	virtual ~AUTO_DELETE_TEMPLATE()
	{
		for (vector<T>::iterator it = vec_delete_list.begin(); it != vec_delete_list.end(); it++)
		{
			if ((*it) != NULL)
			{
				delete[] (*it);
			}
		}
		vec_delete_list.clear();
	}
	void push_back(T pdelete)
	{
		vec_delete_list.push_back(pdelete);
	}
};