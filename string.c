#include "string.h"

// INTERNAL

void clearStrR(string s)
{
    if (s == NULL)
        return;

    clearStrR(s->next);
    
    if (s->arr != NULL)
        free(s->arr);

    free(s);
}

unsigned long lenChararr(char *arr)
{
    unsigned long l = 0;

    while(arr[l] != '\0')
        l++;

    return l;
}

// EXTERNAL

string str_new()
{
    string s = (string) malloc(sizeof(struct str));

    if (s == NULL)
        return NULL;

    s->next = NULL;
    s->arr = (char *) malloc(STR_MAXLEN * sizeof(char));
    s->arr[0] = '\0';
    s->len = 1;

    return s;
}

void str_del(string s)
{
    clearStrR(s->next);

    if (s->arr != NULL)
        free(s->arr);

    free(s);
}

void str_clear(string s)
{
    clearStrR(s->next);
    s->next = NULL;

    s->arr[0] = '\0';
    s->len = 1;
}

unsigned long str_len(string s)
{
    unsigned l = 0;
    while (s != NULL) {
        l += s->len;
        s = s->next;
    }

    return l;
}

void str_put(string s, char *arr)
{
    str_clear(s);
    str_append(s, arr);
}

void str_put_char(string s, char c)
{
    char arr[2] = {c, '\0'};
    str_put(s, arr);
}

void str_append(string s, char *arr)
{
    unsigned long arrL = lenChararr(arr);
    unsigned long strL = str_len(s) + arrL;

    unsigned int noi = strL / STR_MAXLEN;
    if (strL % STR_MAXLEN != 0)
        noi++;

    string prev = NULL;
    unsigned j = 0;
    for (unsigned i = 0; i < noi; i++) {

        if (s == NULL) {
            s = str_new();
        } else if (s->len == STR_MAXLEN) {
            s = s->next;
            continue;
        }

        if (prev != NULL)
            prev->next = s;

        unsigned int l = s->len - 1;
        for (; l < (strL - i * STR_MAXLEN) && l < STR_MAXLEN; l++) {
            s->arr[l] = arr[j++];
        }
       
        s->len = l; 
        prev = s;
        s = s->next;
    }
}

void str_append_char(string s, char c)
{
    char arr[2] = {c, '\0'};
    str_append(s, arr);
}

void str_concat(string s1, string s2)
{
    char *arr = str_to_array(s2);
    str_append(s1, arr);
    free(arr);
}

void str_println(string s, FILE *f)
{
    while (s != NULL) {
        for (unsigned int i = 0; i < s->len; i++)
            fputc(s->arr[i], f);

        s = s->next;
    }

    fputc('\n', f); 
}

char *str_to_array(string s)
{
    char *arr = (char *) malloc(str_len(s) * sizeof(char));
    unsigned long l = 0;
    
    while (s != NULL) {
        for (unsigned int i = 0; i < s->len; i++) {
            arr[l++] = s->arr[i];
        }

        s = s->next;
    } 

    return arr;
}

int str_cmp(string s1, string s2)
{
    if (s1 == s2)
        return 0;

    int res = 0;
    long d_len = str_len(s1) - str_len(s2);
    if (d_len < 0)
        res = -1;
    else if (d_len > 0)
        res = 1;
    
    while (res == 0 && s1 != NULL && s2 != NULL) {
        res = strncmp(s1->arr, s2->arr, s1->len);
        s2 = s2->next;
        s1 = s1->next;
    }

    return res;
}