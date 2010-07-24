#include "FifoWork.h"

Work::Work(Callable work, wxString description)
:   mCallable(work)
,   mDescription(description)
{
    VAR_DEBUG(this)(*this);
}

Work::~Work()
{
    VAR_DEBUG(this);
}

void Work::execute() const
{
    VAR_DEBUG(this);
    mCallable();
}

wxString Work::getDescription() const
{
    return mDescription;
}

std::ostream& operator<< (std::ostream& os, const WorkPtr& obj)
{
    os << obj->getDescription();
    return os;
}

std::ostream& operator<< (std::ostream& os, const Work& obj)
{
    os << obj.getDescription();
    return os;
}

//#include <boost/function.hpp>
//#include <boost/bind.hpp>
//#include <iostream>
//#include <vector>
//
//class CClass1
//{
//public:
//    void AMethod(int i, float f) { std::cout << "CClass1::AMethod(" << i <<");\n"; }
//};
//
//class CClass2
//{
//public:
//    void AnotherMethod(int i) { std::cout << "CClass2::AnotherMethod(" << i <<");\n"; }
//};
//
//int main() {
//    boost::function< void (int) > method1, method2;
//    CClass1 class1instance;
//    CClass2 class2instance;
//    method1 = boost::bind(&CClass1::AMethod, class1instance, _1, 6.0) ;
//    method2 = boost::bind(&CClass2::AnotherMethod, class2instance, _1) ;
//
//    // does class1instance.AMethod(5, 6.0)
//    method1(5);
//
//    // does class2instance.AMethod(5)
//    method2(5);
//
//
//    // stored in a vector of functions...
//    std::vector< boost::function<void(int)> > functionVec;
//    functionVec.push_back(method1);
//    functionVec.push_back(method2);
//
//    for ( int i = 0; i < functionVec.size(); ++i)
//    {         
//    	 functionVec[i]( 5);
//    };
//    return 0;
//};
//
