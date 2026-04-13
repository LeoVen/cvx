#include <stdio.h>

static int MIN_NUMBER = 1000;
static int MAX_NUMBER = 9999;
static int MAX_ITER = 16;

int char_comp(char a, char b)
{
    return a - b;
}

#define V char
#define SNAME string
#define PFX s
#define TAG 1
#include "cvx/dynamic_array.h"
typedef struct string string;
typedef struct string *string_ptr;
static struct string_vtabv *string_vtab = &(struct string_vtabv){
    .comp = char_comp,
};

#define V struct string *
#define SNAME entry_list
#define PFX l
#define TAG 10
#include "cvx/dynamic_array.h"
typedef struct entry_list entry_list;
typedef struct entry_list *entry_list_ptr;
static struct entry_list_vtabv *entry_vtab = &(struct entry_list_vtabv){
    .drop = s_drop,
    .comp = s_compare,
};

int parse_int(string_ptr s);
string_ptr to_string(int number);

void to_c_string(string_ptr value, char *out, size_t max_len);
void print_entry(struct entry_list *entry);

void kaprekar(struct entry_list *entries);

int main(void)
{
    for (int i = MIN_NUMBER; i <= MAX_NUMBER; i++)
    {
        struct entry_list entries = l_init(entry_vtab);
        l_push_back(&entries, to_string(i));

        kaprekar(&entries);

        print_entry(&entries);

        l_clear(&entries);
    }

    return 0;
}

void kaprekar(struct entry_list *entries)
{
    // at least the first number must be in
    if (l_count(entries) <= 0)
    {
        return;
    }

    for (int i = 0; i < MAX_ITER; i++)
    {
        string_ptr last = l_back(entries);

        // make two copies of original, one sorted, one reverse
        string_ptr ascending = s_clone(last);
        s_sort(ascending); // digits are in ascending order

        string_ptr descending = s_clone(ascending);
        for (size_t si = 0; si < s_count(descending) / 2; si++)
        {
            s_swap(descending, si, s_count(descending) - si - 1);
        }

        int higher = parse_int(descending);
        int smaller = parse_int(ascending);
        int result = higher - smaller;

        s_drop(ascending);
        s_drop(descending);

        string_ptr res_str = to_string(result);
        // pad left
        while (s_count(res_str) < s_count(last))
            s_push_front(res_str, '0');

        // Early stop
        if (s_compare(last, res_str) == 0)
            s_drop(res_str);
        else
            l_push_back(entries, res_str);
    }
}

string_ptr to_string(int number)
{
    string_ptr res = s_new_with(string_vtab, 16);

    if (number == 0)
    {
        s_push_back(res, '0');
        return res;
    }

    // Build digits in reverse, then reverse in place
    int n = number < 0 ? -number : number;
    size_t start = 0;

    if (number < 0)
    {
        s_push_back(res, '-');
        start = 1;
    }

    while (n > 0)
    {
        s_push_back(res, '0' + (n % 10));
        n /= 10;
    }

    // Reverse the digit portion
    size_t end = s_count(res) - 1;
    while (start < end)
    {
        s_swap(res, start, end);
        start++;
        end--;
    }

    return res;
}

int parse_int(string_ptr s)
{
    int num = 0;
    for (size_t i = 0; i < s_count(s); i++)
    {
        char d = s_get(s, i);
        if (d >= '0' && d <= '9')
        {
            int val = d - '0';
            num = num * 10 + val;
        }
    }
    return num;
}

void to_c_string(string_ptr value, char *out, size_t max_len)
{
    if (s_empty(value))
    {
        strncpy(out, "\"\"", 2);
        return;
    }

    size_t count = s_count(value);
    size_t max = count > max_len ? max_len : count;

    for (size_t i = 0; i < max; i++)
    {
        out[i] = s_get(value, i);
    }

    out[s_count(value)] = '\0';
}

void print_entry(struct entry_list *entry)
{
    size_t max_len = 100;
    char c_str[max_len];

    if (l_empty(entry))
    {
        printf("<empty>");
        return;
    }

    string_ptr s = l_get(entry, 0);
    to_c_string(s, c_str, max_len);
    printf("[%6s]", c_str);

    for (size_t i = 1; i < l_count(entry); i++)
    {
        string_ptr s = l_get(entry, i);
        to_c_string(s, c_str, max_len);

        printf(" -> [%6s]", c_str);
    }

    printf("\n");
}
