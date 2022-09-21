#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <math.h>
#include <string.h>


#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT0TO9(ch)  ((ch)>='0'&&(ch)<='9')
#define ISDIGIT1TO9(ch)  ((ch)>='1'&&(ch)<='9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}


static int lept_parse_literal(lept_context*c, lept_value* v,const char* literal,lept_type type){
    size_t pos;
    EXPECT(c,literal[0]);
    for(pos=0;pos<strlen(literal)-1;pos++){
        if(c->json[pos]!=literal[pos+1]){
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    c->json += pos;
    v->type = type;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* pos=c->json;
    if(*pos=='-'){    
       pos++;    
    }
    if(*pos=='0'){
        pos++;
    }else{
        if(!ISDIGIT1TO9(*pos)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        for(pos++;ISDIGIT0TO9(*pos);pos++);
    }
    if(*pos=='.'){
       pos++;
       if(!ISDIGIT0TO9(*pos)){
        return LEPT_PARSE_INVALID_VALUE;
       } 
       for(pos++;ISDIGIT0TO9(*pos);pos++);
    }
    if(*pos=='e'||*pos=='E'){
        pos++;
        if(!ISDIGIT0TO9(*pos)&&*pos != '+' && *pos != '-'){
            return LEPT_PARSE_INVALID_VALUE;
        }
        for(pos++;ISDIGIT0TO9(*pos);pos++);
    }
    v->n = strtod(c->json, NULL);
    if(v->n==HUGE_VAL||v->n==-HUGE_VAL){
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    v->type = LEPT_NUMBER;
    c->json = pos;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v,"true",LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v,"false",LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v,"null",LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
