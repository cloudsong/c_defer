/**
 * c_defer and c_scope_guard
 * implement defer & scope_guard library for C
 * just keep temp-resource managing easier ...
 * by: cloudsong @ 2024
 * License: MIT
 */


#include "c_defer.h"
#include "c_scopeguard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------

#define my_free(p) \
    printf("line:%d  free(%p)\n", __LINE__, (p)); \
    free(p)


int test_scopeguard() {

    scope_exit({

        printf("1: ___test scope_exit()\n");

        printf("exit!!!\n");
    });

    scope_exit({

        printf("2: ___test scope_exit()\n");

        printf("another callback exit!!!\n");
    });

    {
        char* str1 = strdup("zxvhjwerfqwd");
        scope_exit1(str1, free(str1));

        char* str2 = strdup("zxvhjwerfqwd");
        scope_exit1_ex(str2, {
            printf(" free str2: [%s]...\n", str2);
            free(scope_arg(0));
        });

        char* str3 = strdup("zxvhjwerfqwd");
        scope_exit1_named(a_str, str3, {
            printf(" free str3: [%s]...\n", a_str);
            free(a_str);
        });
    }

    {
        char*mem1 = malloc(100);
        strcpy(mem1, "capture mem1 and access it by scopt_arg(X) ...");

        int id = 10;

        scope_exit2_ex(mem1, id, {
            printf("11: mem1=%p\n", scope_arg(0));
            printf("11: start release: id=%d mem1=[%s]\n", scope_arg(1), scope_arg(0));
            free(scope_arg(0));
        });

        mem1 = malloc(100);
        strcpy(mem1, "capture mem1 and create scope var with same name ...");
        id = 11;
        scope_exit2(mem1, id, {
            printf("22: mem1=%p\n", mem1);
            printf("22: start free id=%d mem1:[%s]\n", id, mem1);
            free(mem1);
        });

        mem1 = malloc(100);
        strcpy(mem1, "capture value and create alias name");
        id = 12;
        scope_exit2_named(my_mem1, mem1, my_id, id, {
            printf("33: mem1=%p\n", my_mem1);
            printf("33: start free id=%d mem1:[%s]\n", my_id, my_mem1);
            free(my_mem1);
        });
    }

    {
        int a = 100;
        const char* a100 = "sdada";
        struct {
            int age;
        } human;

        human.age = 30;

        scope_exit3(a, a100, human, {
            printf("3: ___test scope_exit3\n");

            printf("a = %d\n", a);
            printf("a100=%s\n", a100);
            printf("age = %d\n", human.age);
        });

        scope_exit3_ex(200, (const char*)"literal-str", human, {
            printf("4: ___test scope_exit3_ex\n");

            printf("arg0 = %d\n", scope_arg(0));
            printf("arg1 =%s\n", scope_arg(1));
            printf("age = %d\n", scope_arg(2).age);
        });

        human.age = 40;
    }


    {

        char*mem4 = malloc(100);
        strcpy(mem4, "capture mem4 and access it by scopt_arg(X) ...");

        int id = 40;

        scope_exit4_ex(mem4, id, 1000, 2000, {
            printf("41: mem1=%p\n", scope_arg(0));
            printf("41: start release: id=%d mem1=[%s]\n", scope_arg(1), scope_arg(0));
            printf("arg3=%d   arg4=%d\n", scope_arg(2), scope_arg(3));
            free(scope_arg(0));
        });

        mem4 = malloc(100);
        strcpy(mem4, "capture mem4 and create scope var with same name ...");
        id = 41;
        int xxx_var3 = 4000;
        struct _xxx_type_4 {
            int age;
            const char* name;
        } xxx_var4 = {
            10,
            "name: 11223344zxcsdgyusd"
        };
        scope_exit4(mem4, id, xxx_var3, xxx_var4, {
            printf("42: mem1=%p\n", mem4);
            printf("42: start free id=%d mem1:[%s]\n", id, mem4);
            printf("arg3=%d\n", xxx_var3);
            printf("arg4= { %d , %s }\n", xxx_var4.age, xxx_var4.name);
            free(mem4);
        });

        mem4 = malloc(100);
        strcpy(mem4, "capture value and create alias name");
        id = 42;
        xxx_var3 = 5000;
        xxx_var4.age = 20;
        xxx_var4.name = "vvvccc";
        scope_exit4_named(my_mem4, mem4, my_id, id, my_var3, xxx_var3, my_var4, xxx_var4, {
            printf("43: mem1=%p\n", my_mem4);
            printf("43: start free id=%d mem1:[%s]\n", my_id, my_mem4);
            printf("arg3=%d\n", my_var3);
            printf("arg4= { %d , %s }\n", my_var4.age, my_var4.name);
            free(my_mem4);
        });
    }

    return 0;
}



int test_defer() {

    defer_init(1024, NULL);

    defer({
        printf("0: defer is called!\n");
    });

    const char* s = "123123";
    defer1_ex(s, {
        const char* ss = defer_arg(0);
        printf("1: call defer! s=%s\n", ss);
    });

    s = NULL;
    const char * ss1 = "cccc";

    defer1_named(local_var, ss1, {
        printf("2: my_s = [%s]\n", local_var);
    });

    ss1 = NULL;

    int zz = 100;
    defer1(zz, {
        printf("3: zz = %d\n", zz);
    });

    zz = 1023;

    // test case: closure with 2-values captured
    char* mem1, *mem2;
    int id = 0;
    {
        mem1 = (char*)malloc(100);
        strcpy(mem1, "21: defer with captered-value");
        mem2 = (char*)malloc(100);
        strcpy(mem2, "21: another value");
        id = 21;

        defer2(mem1, mem2, {
            printf("21: free-mem here! [%s] [%s]\n", mem1, mem2);
            my_free(mem1);
            my_free(mem2);
        });

        mem1 = mem2 = NULL;

        mem1 = (char*)malloc(100);
        strcpy(mem1, "22: defer with captered-value and alias name");
        id ++;

        defer2_named(my_mem1, mem1, my_id, id, {
            printf("22: free-mem here! [%d] [%s]\n", my_id, my_mem1);
            my_free(my_mem1);
        });

        mem1 = mem2 = NULL;

        mem1 = (char*)malloc(200);
        strcpy(mem1, "23: defer with 2-value captured, access them by defer_arg(N)...");
        id ++;

        defer2_ex(mem1, 23, {
            printf("23: free-mem here! [%d] [%s]\n", defer_arg(1), defer_arg(0));
            my_free(defer_arg(0));
        });

    }

    // test case
    {
        char* mem1, *mem2;
        int id = 0;

        mem1 = (char*)malloc(100);
        strcpy(mem1, "defer with captered-value");
        mem2 = (char*)malloc(100);
        strcpy(mem2, "another value");
        id = 31;

        defer3(id, mem1, mem2, {
            printf("%d: free-mem here! [%s] [%s]\n", id, mem1, mem2);
            my_free(mem1);
            my_free(mem2);
        });

        mem1 = mem2 = NULL;
        mem1 = (char*)malloc(100);
        strcpy(mem1, "defer with captered-value and alias name");
        id ++;
        struct {
            int id;
            const char* name;
        } info1 = {
            100,
            (const char*)"cloud"
        };

        defer3_named(my_id, id, my_mem1, mem1, my_info, info1, {
            printf("%d: free-mem here! [%s]\n", my_id, my_mem1);
            printf("info: %d [%s]\n", my_info.id, my_info.name);
            my_free(my_mem1);
        });

        mem1 = mem2 = NULL;
        mem1 = (char*)malloc(200);
        strcpy(mem1, "33: defer with 3-value captured, access them by `defer_arg(N)`.");
        id ++;

        defer3_ex(id, mem1, 10000, {
            printf("%d: arg[2]=%d free-mem here! [%s]\n", defer_arg(0), defer_arg(2), defer_arg(1));
            my_free(defer_arg(1));
        });

    }

    // test case
    {
        mem1 = (char*)malloc(100);
        memset(mem1, 0, 100);
        memset(mem1, '3', 10);
        mem2 = (char*)malloc(100);
        memcpy(mem2, mem1, 20);

        defer4_ex(mem1, mem2, 100, (const char*)"sdasdasd", {

            printf("defer4_ex is called!: "
                "mem1=%s\n"
                "mem2=%s\n"
                "id=%d\n"
                "name=%s\n",
                defer_arg(0),
                defer_arg(1),
                defer_arg(2),
                defer_arg(3)
            );
            my_free(defer_arg(0));
            my_free(defer_arg(1));
            printf("dtor is done.\n");
        });

        /// --- ---

        mem1 = (char*)malloc(100);
        strcpy(mem1, "defer with captered-value");
        mem2 = (char*)malloc(100);
        strcpy(mem2, "another value");
        id = 42;
        const char* var_xx = "this is var xxx";

        defer4(id, mem1, mem2, var_xx, {
            printf("%d: free-mem here! [%s] [%s] [%s]\n", id, mem1, mem2, var_xx);
            my_free(mem1);
            my_free(mem2);
        });
        var_xx = NULL;

        /// --- ---

        mem1 = mem2 = NULL;
        mem1 = (char*)malloc(100);
        strcpy(mem1, "22: defer with captered-value and alias name");
        id ++;
        struct {
            int id;
            const char* name;
        } info1 = {
            100,
            (const char*)"cloud"
        };

        var_xx = "new-var-xxx";

        defer4_named(my_id, id, my_mem1, mem1, my_info, info1, my_var_xxx, var_xx, {
            printf("%d: free-mem here! [%s]\n", my_id, my_mem1);
            printf("info: %d [%s] [%s]\n", my_info.id, my_info.name, my_var_xxx);
            my_free(my_mem1);
        });

    }

    printf("mem1 = [%s]\n", mem1);
    printf("mem2 = [%s]\n", mem2);

    printf("-----\n");

    // closure_mgr_release(&__defer_mgr.base);

    return 0;
}


int main() {

    test_scopeguard();

    test_defer();

    return 0;
}