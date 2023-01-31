/*
 * Challenge (by https://www.youtube.com/@AnotherRoof ):
 * 
 * So to clarify, I want to see a list of the percentage of solvable games
 * for ALL options of large numbers. Like I did for the 15 options
 * of the form {n, n+25, n+50, n+75}, but for all of them.
 * The options for large numbers should be four distinct numbers
 * in the range from 11 to 100. As I said there are 2,555,190 such options
 * so this will require a clever bit of code, but I think it’s possible!
 * Email me via my website if you think you have it!
 */

/*
 * countdown.c
 * Author: "Cheos" <cheos@cheos.dev>
 * Date: 27.01.2023
 * ref: https://youtu.be/X-7Wev90lw4
 * THIS FILE CONTAINS SOME COOL STUFF AND IS FULLY RUNNABLE, BUT IS NOT AS OPTIMIZED,
 * AS countdown_clean.c IS!
 * 
 * This implementation cuts short on some of the verbosity of the original script,
 * as it is only relevant whether a solution exists or not - we do NOT need to find
 * all possible calculations to reach a certain solution value.
 * 
 * I'm 100% sure there'll be faster implementations than this - I'm not
 * all too used to optimizing problems using uncommon algorithms.
 * What i can do though is write some (probably) rather efficient code...
 * Why C? Cuz it's fast!
 * This implementation is purely single-threaded, a 16x speed-up
 * (or more, depending on the amount of cores) could be achieved using multiple threads/processes.
 * I probably missed some really simple optimizations, too - so don't expect this to run FAST,
 * it's just FAST ENOUGH... probably... hopefully.....
 * 
 * Well... to make this compute within 1 day, it'd have to take 33.8ms MAX to compute for each set of 'large' numbers...
 * 
 * Building:
 * 
 * [windows]:
 * install gcc using mingw or cygwin (only standard libs (stdlib) required) 
 * from cmd, or powershell, execute:
 * gcc -o countdown.exe -Ofast countdown.c
 * ./countdown
 * 
 * [linux/wsl]
 * install gcc using your favourite package manager (apt, pacman, ...)
 * from bash (or whatever shell you prefer), execute:
 * gcc -o countdown -Ofast countdown.c
 * ./countdown
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

const size_t sizeof_int = sizeof(int);

// typedef struct list {
//     int size;
//     int *val;
// } list;

typedef struct llnode {
    void* prev;
    void* next;
    int val;
} llnode;

typedef struct linkedlist {
    llnode* first;
    llnode* last;
    size_t size;
} linkedlist;

typedef int (*iiif)(int, int);

int f_add(int a, int b) { return a + b; }
int f_sub(int a, int b) { return a - b; }
int f_mul(int a, int b) { return a * b; }
int f_div(int a, int b) { return a / b; }
iiif opts[] = { &f_add, &f_sub, &f_mul, &f_div };

void printll(linkedlist* ll) {
    printf("[");
    if (ll->size > 0) {
        llnode* node = ll->first;
        printf("%d", node->val);
        for (int i = 1; i < ll->size; i++) {
            node = node->next;
            printf(", %d", node->val);
        }
    }
    printf("]\n");
}

void printll_dbg(linkedlist* ll) {
    printf("[\n");
    if (ll->size > 0) {
        llnode* node = ll->first;
        printf("(%p <- %p{%d} -> %p)\n", node->prev, node, node->val, node->next);
        for (int i = 1; i < ll->size; i++) {
            node = node->next;
            printf(", (%p <- %p{%d} -> %p)\n", node->prev, node, node->val, node->next);
        }
    }
    printf("]\n");
}

llnode* mknode(int val) {
    llnode* node = malloc(sizeof(llnode));
    node->next = node->prev = NULL;
    node->val = val;
    return node;
}

int count_nz_then_clear(int* set, int start, size_t size) {
    int count = 0;
    for (int i = start; i < size; i++) {
        if (set[i]) count++;
        set[i] = 0;
    }
    return count;
}

void clearll(linkedlist* ll) {
    for (llnode* node = ll->first; node != NULL; /*node = node->next*/) {
        llnode* next = node->next;
        if (node == next) puts("something went terribly wrong!");
        free(node);
        node = next;
    }
    ll->size = 0;
}

void freell(linkedlist* ll) {
    clearll(ll);
    free(ll);
}

linkedlist* asll6(int v1, int v2, int v3, int v4, int v5, int v6) {
    linkedlist* ll = malloc(sizeof(linkedlist));
    ll->size = 6;
    llnode* prev;
    llnode* node = mknode(v1);
    ll->first = node;
    ll->last = node;

    node = mknode(v2);
    if (v1 > v2) {
        ll->first->next = node;
        node->prev = ll->first;
        ll->last = node;
    } else {
        ll->last->prev = node;
        node->next = ll->last;
        ll->first = node;
    }

    int v[] = { v3, v4, v5, v6 };
    for (int i = 0; i < 4; i++) {
        node = mknode(v[i]);
        for (prev = ll->first; prev != NULL && prev->val < node->val; prev = prev->next);
        if (prev == NULL) { // insert as last
            node->prev = ll->last;
            ll->last->next = node;
            ll->last = node;
        } else if (prev->val < node->val) { // insert as first
            node->next = ll->first;
            ll->first->prev = node;
            ll->first = node;
        } else { // insert in center
            llnode* next = prev->next;
            next->prev = node;
            node->next = next;
            prev->next = node;
            node->prev = prev;
        }
    }
    return ll;
}

linkedlist* asll(int* vals, size_t size) {
    linkedlist* ll = malloc(sizeof(linkedlist));
    ll->size = size;
    if (size == 0) return ll;
    ll->first = mknode(vals[0]);

    llnode* prev = ll->first;
    for (int i = 1; i < size; i++) {
        llnode* node = mknode(vals[i]);
        prev->next = node;
        node->prev = prev;
        prev = node;
    }
    ll->last = prev;
    return ll;
}

linkedlist* copyll_rem_ins(linkedlist* ll, int remidx1, int remidx2, int insval) {
    if (ll->size <= 2) return asll(&insval, 1);
    linkedlist* out = malloc(sizeof(linkedlist));
    out->size = ll->size - 1;
    llnode* llcur = ll->first;
    llnode* llprev = NULL;
    llnode* outprev = NULL;

    for (int i = 0; i < ll->size; i++, llcur = llcur->next) {
        if (i == remidx1 || i == remidx2) continue;
        if ((llprev == NULL || (llprev->val >= insval)) && llcur->val < insval) { // insert as first or in center
            llnode* node = mknode(insval);
            if (outprev == NULL)
                out->first = node;
            else {
                outprev->next = node;
                node->prev = outprev;
            }
            outprev = node;
        }

        llnode* node = mknode(llcur->val);
        if (outprev == NULL)
            out->first = node;
        else {
            outprev->next = node;
            node->prev = outprev;
        }
        outprev = node;
        llprev = llcur;
    }

    if (outprev->val >= insval) { // insert as last
        llnode* node = mknode(insval);
        outprev->next = node;
        node->prev = outprev;
        outprev = node;
    }
    out->last = outprev;
    return out;
}

int* acopy(int* arr, int size) {
    int* out = malloc(sizeof_int * size);
    int i, tmp;
    for (int i = 0; i < size; i++)
        out[i] = arr[i];

    return out;
}

void sort_desc(int* arr, size_t size) {
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            if (arr[i] > arr[j]) {
                int tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
            }
}

int div_err(int dividend, int divisor) {
    return (dividend < divisor) || ((dividend % divisor) != 0);
}

int* arr_remove(int* arr, size_t size, size_t res_size, int idxa, int idxb) {
    int *out = malloc(sizeof_int * res_size);
    for (int i = 0, j = 0; i < size && j < res_size; i++, j++)
        if (i == idxa || i == idxb) j--;
        else out[j] = arr[i];
    return out;
}

int* arr_remove_val(int* arr, size_t size, size_t res_size, int val) {
    int *out = malloc(sizeof_int * res_size);
    for (int i = 0, j = 0; i < size && j < res_size; i++, j++)
        if (arr[i] == val) j--;
        else out[j] = arr[i];
    return out;
}

char* prepend_opt(char* suffix, int a, int b, int res, int opt) {
    char *out = malloc(sizeof(char) * 1024);
    char buf[128];
    switch (opt) {
        case 0: sprintf(buf, "%d + %d = %d", a, b, res); break;
        case 1: sprintf(buf, "%d - %d = %d", a, b, res); break;
        case 2: sprintf(buf, "%d * %d = %d", a, b, res); break;
        case 3: sprintf(buf, "%d / %d = %d", a, b, res); break;
        default: sprintf(buf, "%d ? %d = %d", a, b, res);
    }
    sprintf(out, "%s, %s", buf, suffix);
    return out;
}

char* solve(int *set, size_t size, int sol) { // not optimized!
    if (size < 2) return NULL;
    sort_desc(set, size);
    for (int i = 0; i < size; i++)
        for (int j = i + 1; j < size; j++)
            for (int op = 0; op < 4; op++) {
                int* mut = acopy(set, size);
                int a = mut[i], b = mut[j];
                int res = opts[op](a, b);
                if (res <= 0 || div_err(a, b)) {
                    free(mut);
                    continue;
                }
                if (res == sol) {
                    free(mut);
                    return prepend_opt("\n", a, b, res, op);
                }
                int* mut_new = arr_remove(mut, size, size - 1, i, j);
                free(mut);
                mut = mut_new;
                mut[size - 2] = res;
                
                char* str = solve(mut, size - 1, sol);
                free(mut);
                if (str == NULL) continue;
                char* ret = prepend_opt(str, a, b, res, op);
                free(str);
                return ret;
            }
    return NULL;
}

void solution_set(int* sols, linkedlist* set) {
    if (set->size < 2) return;
    llnode* an = set->first;
    for (int i = 0; i < set->size - 1; i++, an = an->next) {
        llnode* bn = an->next;
        for (int j = i + 1; j < set->size; j++, bn = bn->next) {
            for (int op = 0; op < 4; op++) {
                int a = an->val;
                int b = bn->val;
                int res = opts[op](a, b);

                if ((res <= 0) || div_err(a, b)) continue;
                if (res > 99 && res < 1000) { // insertll_after_ifnexist(sols, res);
                    sols[0]++;
                    sols[res]++;
                }
                if (set->size == 2) continue; // no need to compute the following if no results will come off it

                linkedlist* mut = copyll_rem_ins(set, i, j, res);
                solution_set(sols, mut);
                freell(mut);
            }
        }
    }
}

void iterate_sets() {
    int valid_smalls[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int* solset = malloc(sizeof(int) * 1024);

    long long set_0_large_count, set_1_large_count = 0, set_2_large_count = 0,
            set_3_large_count = 0, set_4_large_count = 0;
    long long sol_0_large_count, sol_1_large_count = 0, sol_2_large_count = 0,
            sol_3_large_count = 0, sol_4_large_count = 0;

    int larges[] = { 25, 50, 75, 100 }; // TODO iter

    // // 0 "larges"
    // // 3 pairs
    // for (int i = 1; i < 11; i++)
    //     for (int j = i + 1; j < 11; j++)
    //         for (int k = j + 1; k < 11; k++) {
    //             int set[] = { i, i, j, j, k, k };
    //             solution_set(solset, set, 6);
    //             set_0_large_count++;
    //             sol_0_large_count += solset->size;
    //             solset->size = 0;
    //         }
    // // 2 pairs
    // for (int i = 1; i < 11; i++)
    //     for (int j = i + 1; j < 11; j++)
    //         for (int k = 1; k < 11; k++) {
    //             if (k == i || k == j) continue;
    //             for (int l = k + 1; l < 11; l++) {
    //                 if (l == i || l == j) continue;
    //                 int set[] = { i, i, j, j, k, l };
    //                 solution_set(solset, set, 6);
    //                 set_0_large_count++;
    //                 sol_0_large_count += solset->size;
    //                 solset->size = 0;
    //             }
    //         }
    // // 1 pair
    // for (int i = 1; i < 11; i++) {
    //     int *mut = arr_remove_val(valid_smalls, 10, 9, i);
    //     for (int j = 0; j < 6; j++) 
    //         for (int k = j + 1; k < 7; k++)
    //             for (int l = k + 1; l < 8; l++)
    //                 for (int m = l + 1; m < 9; m++) {
    //                     int set[] = { i, i, mut[j], mut[k], mut[l], mut[m] };
    //                     solution_set(solset, set, 6);
    //                     set_0_large_count++;
    //                     sol_0_large_count += solset->size;
    //                     solset->size = 0;
    //                 }
    // }
    // // 0 pairs
    // for (int i = 1; i < 6; i++)
    //     for (int j = i + 1; j < 7; j++)
    //         for (int k = j + 1; k < 8; k++)
    //             for (int l = k + 1; l < 9; l++)
    //                 for (int m = l + 1; m < 10; m++)
    //                     for (int n = m + 1; n < 11; n++) {
    //                         int set[] = { i, j, k, l, m, n };
    //                         solution_set(solset, set, 6);
    //                         set_0_large_count++;
    //                         sol_0_large_count += solset->size;
    //                         solset->size = 0;
    //                     }
    printf("20%% (computed sets with 0 larges)\n");
    
    // 1 "large"
    for (int L = 0; L < 4; L++) {
        // 2 pairs
        for (int i = 1; i < 10; i++)
            for (int j = i + 1; j < 11; j++)
                for (int k = 1; k < 11; k++) {
                    if (k == i || k == j) continue;
                    linkedlist* set = asll6(larges[L], i, i, j, j, k);
                    solution_set(solset, set);
                    set_1_large_count++;
                    sol_1_large_count += count_nz_then_clear(solset, 99, 1000);
                }
        // 1 pair
        for (int i = 1; i < 11; i++) {
            int *mut = arr_remove_val(valid_smalls, 10, 9, i);
            for (int j = 0; j < 9; j++)
                for (int k = j + 1; k < 9; k++)
                    for (int l = k + 1; l < 9; l++) {
                        linkedlist* set = asll6(larges[L], i, i, mut[j], mut[k], mut[l]);
                        solution_set(solset, set);
                        set_1_large_count++;
                        sol_1_large_count += count_nz_then_clear(solset, 99, 1000);
                    }
        }
        // 0 pairs
        for (int i = 1; i < 11; i++)
            for (int j = i + 1; j < 11; j++)
                for (int k = j + 1; k < 11; k++)
                    for (int l = k + 1; l < 11; l++)
                        for (int m = l + 1; m < 11; m++) {
                            linkedlist* set = asll6(larges[L], i, j, k, l, m);
                            solution_set(solset, set);
                            set_1_large_count++;
                            sol_1_large_count += count_nz_then_clear(solset, 99, 1000);
                        }
    }
    printf("40%% (computed sets with 1 large)\n");

    // 2 "larges"
    for (int L1 = 0; L1 < 4; L1++)
        for (int L2 = L1 + 1; L2 < 4; L2++) {
            // 2 pairs
            for (int i = 1; i < 11; i++)
                for (int j = i + 1; j < 11; j++) {
                    linkedlist* set = asll6(larges[L1], larges[L2], i, i, j, j);
                    solution_set(solset, set);
                    set_2_large_count++;
                    sol_2_large_count += count_nz_then_clear(solset, 99, 1000);
                }
            // 1 pair
            for (int i = 1; i < 11; i++) {
                int *mut = arr_remove_val(valid_smalls, 10, 9, i);
                for (int j = 0; j < 9; j++) 
                    for (int k = j + 1; k < 9; k++) {
                        linkedlist* set = asll6(larges[L1], larges[L2], i, i, mut[j], mut[k]);
                        solution_set(solset, set);
                        set_2_large_count++;
                        sol_2_large_count += count_nz_then_clear(solset, 99, 1000);
                    }
                free(mut);
            }
            // 0 pairs
            for (int i = 1; i < 11; i++)
                for (int j = i + 1; j < 11; j++)
                    for (int k = j + 1; k < 11; k++)
                        for (int l = k + 1; l < 11; l++) {
                            linkedlist* set = asll6(larges[L1], larges[L2], i, j, k, l);
                            solution_set(solset, set);
                            set_2_large_count++;
                            sol_2_large_count += count_nz_then_clear(solset, 99, 1000);
                        }
        }
    printf("60%% (computed sets with 2 larges)\n");
    
    // 3 "larges"
    for (int L1 = 0; L1 < 4; L1++)
        for (int L2 = L1 + 1; L2 < 4; L2++)
            for (int L3 = L2 + 1; L3 < 4; L3++) {
                // 1 pair
                for (int i = 1; i < 11; i++) {
                    int *mut = arr_remove_val(valid_smalls, 10, 9, i);
                    for (int j = 0; j < 9; j++) {
                        linkedlist* set = asll6(larges[L1], larges[L2], larges[L3], i, i, mut[j]);
                        solution_set(solset, set);
                        set_3_large_count++;
                        sol_3_large_count += count_nz_then_clear(solset, 99, 1000);
                    }
                    free(mut);
                }
                // 0 pairs
                for (int i = 1; i < 11; i++)
                    for (int j = i + 1; j < 11; j++)
                        for (int k = j + 1; k < 11; k++) {
                            linkedlist* set = asll6(larges[L1], larges[L2], larges[L3], i, j, k);
                            solution_set(solset, set);
                            set_3_large_count++;
                            sol_3_large_count += count_nz_then_clear(solset, 99, 1000);
                        }
            }
    printf("80%% (computed sets with 3 larges)\n");

    // 4 "larges"
    for (int L1 = 0; L1 < 4; L1++)
        for (int L2 = L1 + 1; L2 < 4; L2++)
            for (int L3 = L2 + 1; L3 < 4; L3++)
                for (int L4 = L3 + 1; L4 < 4; L4++) {
                    // 1 pair
                    for (int i = 1; i < 11; i++) {
                        linkedlist* set = asll6(larges[L1], larges[L2], larges[L3], larges[L4], i, i);
                        solution_set(solset, set);
                        set_4_large_count++;
                        sol_4_large_count += count_nz_then_clear(solset, 99, 1000);
                    }
                    // 0 pairs
                    for (int i = 1; i < 11; i++)
                        for (int j = i + 1; j < 11; j++) {
                            linkedlist* set = asll6(larges[L1], larges[L2], larges[L3], larges[L4], i, j);
                            solution_set(solset, set);
                            set_4_large_count++;
                            sol_4_large_count += count_nz_then_clear(solset, 99, 1000);
                        }
                }
    printf("100%% (computed sets with 4 larges)\n");
    
    // printf("found %d solutions for %d sets with 0 large numbers (%.3f%%)\n", sol_0_large_count, set_0_large_count, 100.0*sol_0_large_count/set_0_large_count);
    printf("found %d solutions for %d sets with 1 large number  (%.3f%%)\n", sol_1_large_count, set_1_large_count, 100.0*sol_1_large_count/set_1_large_count);
    printf("found %d solutions for %d sets with 2 large numbers (%.3f%%)\n", sol_2_large_count, set_2_large_count, 100.0*sol_2_large_count/set_2_large_count);
    printf("found %d solutions for %d sets with 3 large numbers (%.3f%%)\n", sol_3_large_count, set_3_large_count, 100.0*sol_3_large_count/set_3_large_count);
    printf("found %d solutions for %d sets with 4 large numbers (%.3f%%)\n", sol_4_large_count, set_4_large_count, 100.0*sol_4_large_count/set_4_large_count);
    printf("found %d total solutions for %d total sets (%.3f%%)\n", solset[0], set_1_large_count + set_2_large_count + set_3_large_count + set_4_large_count, 100.0*solset[0]/(set_1_large_count + set_2_large_count + set_3_large_count + set_4_large_count));
    free(solset);
}

int main(int argc, char *argv[]) {
    clock_t start = clock();
    // -- test set: --
    // int set[] = { 100, 75, 50, 25, 5, 2 };
    // int* mut = acopy(set, 6);
    // for (int i = 0; i < 6; i++)
    //     printf("%d ", mut[i]);
    // printf("\n");
    // return 0;
    //
    // -- test solve + print resulting calculation: --
    // char* str = solve(set, 6, 150);
    // if (str != 0) printf(str);
    // free(str);
    //
    // -- test evaluation of set of reachable solutions: --
    // list* solset = malloc(sizeof(list));
    // solset->size = 0;
    // solset->val = malloc(sizeof(int) * 1024);
    // solution_set(solset, set, 6);
    // printf("[");
    // for (int i = 0; i < solset->size; i++)
    //     printf("%d%s", solset->val[i], i == solset->size - 1 ? "" : ", ");
    // printf("] (%d)\n", solset->size);
    //
    // -- iterate over all possible sets: --
    iterate_sets();

    printf("took %.3fs to compute\n", (clock() - start) * 1.0 / CLOCKS_PER_SEC);
    return 0;
}
