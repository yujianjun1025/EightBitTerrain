#include "EightBitMap.h"
#include <stdexcept>

int main()
{
	try{
		EightBitMap test("orign.raw");
		test.AllDirectionToGradual();
		test.BounaryColToMid();
		test.LetMapSmooth(20);
		test.SaveToFile("end.raw");
	}
	catch(exception&  e)
	{
		printf("%s",e.what());
	}
	 
	return 0;
}
