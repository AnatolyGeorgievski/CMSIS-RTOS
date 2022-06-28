#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
/*
static struct envVars {
	size_t nameLen;
	size_t valueSize;
	char *name;
	char *value;
	bool active;
	bool putenv;
} *envVars = NULL;
static char * __findenv_environ(const char *name, size_t nameLen)
{
	int envNdx;
	for (envNdx = 0; environ[envNdx] != NULL; envNdx++)
		if (strncmpeq(environ[envNdx], name, nameLen))
			return (&(environ[envNdx][nameLen + sizeof("=") - 1]));

	return (NULL);
} 
*/
extern char **environ;// = NULL; NULL terminated list

static inline bool
strncmpeq(const char *nameValue, const char *name, size_t len)
{
	return strncmp(nameValue, name, len)==0 && nameValue[len]=='=';
}

extern char *getenv(const char *name)
{
	if (environ==NULL || name==NULL) return NULL;
	size_t len = strlen(name);
	char** env = environ;
	for (;*env !=NULL; ++env) {
		if (strncmpeq(*env, name, len)){
			return *env + len + 1;
		}
	}
	return NULL;
}
