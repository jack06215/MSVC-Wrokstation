#include <iostream>
#include <algorithm>
#include <vector>

std::vector<int>::iterator toggleIndex(std::vector<int> &src, std::vector<int> &position);

int main(void)
{
	std::vector<int> myList = {1, 44, 66, 88, 77, 553};
	std::vector<int> removeIndex = {0, 1, 3, 4};
	/*std::cout << "Before remove: " << std::endl;
	std::cout << "Size: "<< myList.size() << std::endl;
	for (std::vector<int>::iterator iter = myList.begin(); iter != myList.end(); iter++)
		std::cout << *iter << " ";

	std::cout << "" << std::endl;*/

	myList.erase(toggleIndex(myList, removeIndex), myList.end());

	/*std::cout << "After remove: " << std::endl;
	std::cout << "Size: " << myList.size() << std::endl;
	for (std::vector<int>::iterator iter = myList.begin(); iter != myList.end(); iter++)
		std::cout << *iter << " ";

	std::cout << "" << std::endl;*/

	return 0;
}


std::vector<int>::iterator toggleIndex(std::vector<int> &src, std::vector<int> &position)
{
	position.push_back(-1);							// to handle range from 0 to the first index to remove
	position.push_back(src.size());					// to handle range from the last index to remove and to the end of values
	std::sort(position.begin(), position.end());
	std::vector<int>::iterator last = src.begin();
	for (size_t i = 1; i != position.size(); ++i) 
	{
		size_t range_begin = position[i - 1] + 1;
		size_t range_end = position[i];
		//std::cout << range_begin << "\t" << range_end << "\t" << range_end - range_begin << std::endl;
		std::copy(src.begin() + range_begin, src.begin() + range_end, last);
		last += range_end - range_begin;
		std::cout << *last << std::endl;
	}
	return last;
}