#pragma once
#include <vector>
using std::vector;
class AUTO_DELETE_POINT
{
public:
	vector<void*> vec_point;
	virtual ~AUTO_DELETE_POINT()
	{
		for (vector<void*>::iterator it = vec_point.begin(); it != vec_point.end(); it++)
		{
			if (*it != NULL)
			{
				delete[] * it;
			}
		}
		vec_point.clear();
	}
	void push_back(void* pbuffer)
	{
		vec_point.push_back(pbuffer);
	}
};