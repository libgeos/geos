#ifndef GEOS_UNLOAD_H
#define GEOS_UNLOAD_H
//xie add for realse static memory 2003,10,06
namespace geos {

class Unload
{
public:
	Unload(void);
	~Unload(void);
	static void Release();
};

}
#endif
