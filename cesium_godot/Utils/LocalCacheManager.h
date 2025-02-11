#ifndef LOCAL_CACHE_MANAGER_H
#define LOCAL_CACHE_MANAGER_H

#include <cstdint>

class LocalCacheManager {

public:
	void write_render_resource(uint8_t* data, uint32_t size, size_t hashedId);

private:

};

#endif // LOCAL_CACHE_MANAGER_H
