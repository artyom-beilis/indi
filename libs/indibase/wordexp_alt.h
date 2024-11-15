#pragma once
#ifndef __ANDROID__
#include <wordexp.h>
#else
#include <string.h>
#include <stdio.h>
namespace {
    typedef struct {
        size_t we_wordc;
        char *we_wordv[1];
    } wordexp_t;

    inline int wordexp(char const *src,wordexp_t *res,int )
    {
        res->we_wordc = 1;
        char *home = getenv("HOME");
        // Handle common case of ~
        if(strstr(src,"~/") == src && home != nullptr) {
            size_t len = strlen(home) + strlen(src + 1) + 1;
            char *new_str = (char *)malloc(len);
            if(!new_str)
                return 1;
            strcpy(new_str,home);
            strcat(new_str,src + 1);
            res->we_wordv[0] = new_str;
        }
        else {
            res->we_wordv[0] = strdup(src);
        }
        if(!res->we_wordv[0]) {
            fprintf(stderr,"wordexp [%s] Failed\n",src);
            return 1;
        }
        fprintf(stderr,"wordexp [%s] -> [%s]\n",src,res->we_wordv[0]);
        return 0;
    }
    inline void wordfree(wordexp_t *res)
    {
        res->we_wordc = 0;
        free(res->we_wordv[0]);
    }
}
#endif
