#ifndef ENTITY_H
#define ENTITY_H
#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned long long EntityName;
constexpr const EntityName DecodeName(const char* name) {
	//FNV-1a hashing algorithm
	int len = 0;
	//basic strlen
	for (int i = 0; i < 100; i++) {
		if (name[i] == 0) {
			len = i;
			break;
		}
	}
	if (len == 0)
		return 0;
	EntityName hash = 14695981039346656037;
	EntityName FNV_prime = 1099511628211;

	for (int i = 0; i < len; i++) {
		hash ^= name[i];
		hash *= FNV_prime;
	}

	return hash;
}

#ifdef __cplusplus
}
#endif


#endif