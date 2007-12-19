#ifndef _leak_check_
#define _leak_check_

namespace MemoryLeaks
{
	

class Object
{
public:
	char* file;
	int line;
	
	Object(char* f, int l);
	void print();
};

void checkForLeaks();
void setLocation(char* f, int i);

class LeakCheck
{
	Object* myObj;
public:
	void setLocation(char* file, int line);
	LeakCheck();	
	~LeakCheck();


};

class LeakSafe
{
	MemoryLeaks::LeakCheck myLeakCheck;
public:
	LeakSafe(char* file, int line)
	{
		myLeakCheck.setLocation(file,line);
	}
};

}

#ifdef _CHECK_FOR_LEAKS
#define DECLARE_LEAK_CHECK() MemoryLeaks::LeakCheck myLeakCheck
#define INIT_LEAK_CHECK() myLeakCheck.setLocation(__FILE__,__LINE__);

#define DECLARE_LEAK_SAFE public MemoryLeaks::LeakSafe
#define INIT_LEAK_SAFE MemoryLeaks::LeakSafe(__FILE__,__LINE__)
#endif

#endif
