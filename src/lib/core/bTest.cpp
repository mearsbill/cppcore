
extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include "abcNodes.h"
#include "abcList.h"


class btest_c
{
  private:
  	int64_t a;
	int64_t b;
	char	*name;
  protected:

  public:
  	btest_c(const char *str); // {  name=strdup(str); }
	~btest_c(); //{ free (name); }

  	void puta(int64_t setval) {  a = setval; }
  	void putb(int64_t setval) {  b = setval; }
	void display() { printf("%s a=%lld b=%lld\n",name,a,b); }
	int64_t getaplusb() { return a+b; }
};

btest_c::btest_c(const char *str)
{
	name=strdup(str); 
}
btest_c::~btest_c()
{
	free (name); 
}

int main(int argc, char *argv[])
{
	printf("Hello World\n");

	abcListNode_c *le = new abcListNode_c();
	fprintf(stderr," === \n");
	abcNode_c *otherNode = le;
	abcNode_c *base = new abcNode_c();
	le->setKeyInt(123);
	le->setKeyString("Hello Fred");


	abcListNode_c *le_clone = le->clone();
	delete le_clone;


	abcList_c *myList = new abcList_c("TestList");
	abcList_c *cloneList = myList->clone();
	delete cloneList;

	fprintf(stderr," === deleting le\n");
	delete otherNode;
	fprintf(stderr," === deleting base\n");
	delete base;
	fprintf(stderr," === done \n");

	
	btest_c *bto = new btest_c("Joe");
	bto->puta(123);
	bto->putb(456000);
	bto->display();
	printf("bto= %llx\n",(int64_t)bto);

	delete bto;
}


