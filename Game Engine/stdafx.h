// stdafx.h : file di inclusione per file di inclusione di sistema standard
// o file di inclusione specifici del progetto utilizzati di frequente, ma
// modificati raramente
//

#pragma once

#include "targetver.h"
#include "globals.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <time.h>

#include "transform.h"
//#include "transform_variables.h"	//added

typedef unsigned long long EntityName;

//decode a string name into a EntityName object. The name can be maximum 100 characters
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

// TODO: fare riferimento qui alle intestazioni aggiuntive richieste dal programma
