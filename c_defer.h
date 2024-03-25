/**
 * c_defer
 * implement defer library for C
 * just keep temp-resource managing easier
 * by: cloudsong @ 2024
 * License: MIT
 */

#ifndef __simple_c_defer_h__
#define __simple_c_defer_h__

// -----------------------------------------------------------------------------

/// enable memory failure detect
/// caller can detect memory failure
#if 0
    #define ENABLE_CLOSURE_MEM_FAILURE_DETECT 
#endif

#ifndef ENABLE_CLOSURE_MEM_FAILURE_DETECT
    #include <stdio.h>
    #include <signal.h> // to raise SEGV if closure-stack is low!
#endif

// enable custom closure memory allocator
// user can install custom allocator to alloc defer obj
#define ENABLE_CUSTOM_CLOSURE_ALLOCATOR

struct _defer_closure_head;

/// @brief custom allocator for dyn-defer-closure
typedef struct _defer_closure_allocator {
    /// @brief closure object allocator
    void* (*alloc  )(struct _defer_closure_allocator* self, int size);
    void  (*release)(struct _defer_closure_allocator* self, void* obj);
} defer_closure_allocator_t;

/// @brief closure manager for defer
/// manager and call all registered closure
typedef struct _defer_closure_mgr {
    struct _defer_closure_head* fn_chain; // closure stack
    defer_closure_allocator_t*  allocator;
    int                         builtin_buf_max;
    int                         builtin_buf_used;
    char                        builtin_buff[0];
} defer_closure_mgr_t;

/// @brief closure obj's common head
typedef struct _defer_closure_head {
    struct _defer_closure_head* next;
    void (* callback)(struct _defer_closure_head* self);
#ifdef ENABLE_CUSTOM_CLOSURE_ALLOCATOR
    #define CLOSURE_FLAG_USER_ALLOC (1<<0)
    unsigned long flags; // !!! warning: take care of alignment
#endif
} defer_closure_head_t;


/// @brief alloc, init and push a closure obj to stack
/// @param mgr ptr to closure manager
/// @param size closure obj size
/// @return ptr to new allocated closure obj
static inline defer_closure_head_t* __new_defer_closure(defer_closure_mgr_t*mgr, int size) {
    if (mgr->builtin_buf_used + size < mgr->builtin_buf_max) {
        // alloc
        defer_closure_head_t* out = (defer_closure_head_t*)(mgr->builtin_buff + mgr->builtin_buf_used);
        mgr->builtin_buf_used += size;

        // init
#ifdef ENABLE_CUSTOM_CLOSURE_ALLOCATOR
        out->flags = 0;
#endif
        // push
        out->next = mgr->fn_chain;
        mgr->fn_chain = out;
        return out;
    }

#ifdef ENABLE_CUSTOM_CLOSURE_ALLOCATOR
    // try custom allocator
    if (mgr->allocator) {
        return mgr->allocator->alloc(mgr->allocator, size);
    }
#endif

#ifndef ENABLE_CLOSURE_MEM_FAILURE_DETECT
    printf("*** no-mem for defer-closure, INCREASE `stack_size` of `defer_init(N, A)`!!! userd:%d rest:%d needed:%d\n",
        mgr->builtin_buf_used, mgr->builtin_buf_max - mgr->builtin_buf_used, size
    );
    // panic !!!
    raise(SIGSEGV);
#endif

    return NULL;
}

/// @brief call all closure, then cleanup all
/// @param mgr ptr to closure manager
static inline void __defer_closure_mgr_release(void* _mgr) {
    defer_closure_mgr_t* mgr = (defer_closure_mgr_t*)_mgr;
    defer_closure_head_t* c = mgr->fn_chain;
    defer_closure_head_t* nxt;
    while(c) {
        nxt = c->next;
        // call callback
        c->callback(c);
        // release
#ifdef ENABLE_CUSTOM_CLOSURE_ALLOCATOR
        if (c->flags & CLOSURE_FLAG_USER_ALLOC) {
            mgr->allocator->release(mgr->allocator, c);
        }
#endif
        c = nxt;
    }
}

#define defer_init(stack_size, closure_allocator) \
    __attribute__((cleanup(__defer_closure_mgr_release))) \
    struct _defer_mgr_local { \
        defer_closure_mgr_t base; \
        unsigned char stack[stack_size]; \
    } __defer_mgr = { \
        {NULL, closure_allocator, stack_size, 0} \
    }

#define gen_defer_closure_decl() \
    struct _closure_obj { \
        defer_closure_head_t base

#define gen_defer_closure_field_decl(var_name, cap_val) \
        typeof(cap_val) var_name

#define gen_defer_closure_init() \
    } *__curr_closure = (typeof(__curr_closure)) __new_defer_closure(&__defer_mgr.base, sizeof(*__curr_closure)); \
    if (__curr_closure) {

#define gen_defer_closure_field_init(var_name, cap_val) \
        __curr_closure->var_name = cap_val

#define gen_defer_closure_cb_field_init_part1() \
        __curr_closure->base.callback = (typeof(__curr_closure->base.callback)) ({ \
            void __fn(struct _closure_obj* __curr_closure) {

#define gen_defer_closure_local_var(var_name) \
                typeof(__curr_closure->var_name) var_name = __curr_closure->var_name

#define gen_defer_closure_cb_field_init_part2(code) \
                code ; \
            }; \
            __fn; \
        })

#define gen_defer_end() \
    }\
    __curr_closure ? 1: 0


#define defer_arg(arg_pos) __curr_closure->arg ## arg_pos

// -----------------------------------------------------------------------------

/// register a derfer statement
/// it will be called when exiting the function scope
/// @param code statement that will be called later
/// @return boolean, 0 means memory failed! 1 means ok
#define defer(code) \
({\
    gen_defer_closure_decl(); \
    gen_defer_closure_init()  \
    gen_defer_closure_cb_field_init_part1() \
    gen_defer_closure_cb_field_init_part2(code); \
    gen_defer_end(); \
})

///
/// create a closure that capture value provided by @cap_var1 and register it as defer statement
/// @param cap_val1 is value that is going tobe captured; closure will create a local-var with same name as @cap_val1
///           you can access captured value by closure local var @cap_val1
///           @cap_val1 must be var-token, cannot be expr. if expr is needed, using defer1_ex() instead
/// @param code statement that will be called later
/// @return boolean, 0 means memory failed! 1 means ok
///
/// example:
/// int main() {
///    char* s = malloc(100);
///    if (!s) { return -1; }
///    /// @s is value that closure going tobe captured, it also create a closure local var with same name
///    defer1(s,{ 
///        free(s); // s is closure's local var, save the captured value of `main::s`
///    });
///    return 0;
/// }
///
#define defer1(cap_var1, code) defer1_named(cap_var1, cap_var1, code)

///
/// create a closure that capture value @cap_val1 and register it as defer code
/// @param closure_var1 is closure's local var that save the captured value;
///               you can access captured value by local var @closure_var1
/// @param cap_val1     is value that is going tobe captured
/// @param code statement that will be called later
/// @return boolean, 0 means memory failed! 1 means ok
///
/// example:
/// int main() {
///    char* s = malloc(100);
///    if (!s) { return -1; }
///    /// @my_s is closure's local var that save the captured value 's';
///    /// @s is value that closure going capture
///    defer1_named(s_new_name, s, { 
///        free(s_new_name); // defer_arg(0) is first args of closure
///    });
///    return 0;
/// }
///
#define defer1_named(closure_var1, cap_val1, code) \
({ \
    gen_defer_closure_decl(); \
    gen_defer_closure_field_decl(closure_var1, cap_val1); \
    gen_defer_closure_init()  \
    gen_defer_closure_field_init(closure_var1, cap_val1); \
    gen_defer_closure_cb_field_init_part1() \
    gen_defer_closure_local_var(closure_var1); \
    gen_defer_closure_cb_field_init_part2(code); \
    gen_defer_end(); \
})

///
/// create a closure that capture value provided bt expr @cap_val1 and register it as defer code
/// closure will create local var arg0...argN to save captured values
/// to access closure's local var, using defer_arg(N).
/// @param cap_val1     is value that is going tobe captured
/// @param code statement that will be called later
///
/// example:
/// int main() {
///    char* s = malloc(100);
///    if (!s) { return -1; }
///    defer1_ex(s, { // s is captured value
///        free(defer_arg(0)); // defer_arg(0) is first args of closure
///    });
///    return 0;
/// }
///
#define defer1_ex(cap_val1, code) \
({ \
    gen_defer_closure_decl(); \
    gen_defer_closure_field_decl(arg0, cap_val1); \
    gen_defer_closure_init()  \
    gen_defer_closure_field_init(arg0, cap_val1); \
    gen_defer_closure_cb_field_init_part1() \
    gen_defer_closure_cb_field_init_part2(code); \
    gen_defer_end(); \
})

///
/// @brief capture two values and register a defer statement to handle it
///        same as `defer1_ex`, but more values captured
///
#define defer2_ex(cap_val1, cap_val2, code) \
({ \
    gen_defer_closure_decl(); \
    gen_defer_closure_field_decl(arg0, cap_val1); \
    gen_defer_closure_field_decl(arg1, cap_val2); \
    gen_defer_closure_init()  \
    gen_defer_closure_field_init(arg0, cap_val1); \
    gen_defer_closure_field_init(arg1, cap_val2); \
    gen_defer_closure_cb_field_init_part1() \
    gen_defer_closure_cb_field_init_part2(code); \
    gen_defer_end(); \
})

///
/// @brief capture two values, create alias closure var and register a defer statement to handle it
///        same as `defer1_named`, but more values captured
///
#define defer2_named(closure_var1, cap_val1, closure_var2, cap_val2, code) \
({ \
    gen_defer_closure_decl(); \
    gen_defer_closure_field_decl(closure_var1, cap_val1); \
    gen_defer_closure_field_decl(closure_var2, cap_val2); \
    gen_defer_closure_init()  \
    gen_defer_closure_field_init(closure_var1, cap_val1); \
    gen_defer_closure_field_init(closure_var2, cap_val2); \
    gen_defer_closure_cb_field_init_part1() \
    gen_defer_closure_local_var(closure_var1); \
    gen_defer_closure_local_var(closure_var2); \
    gen_defer_closure_cb_field_init_part2(code); \
    gen_defer_end(); \
})

///
/// @brief capture two values, create alias closure var and register a defer statement to handle it
///        same as `defer1`, but more values captured
///
#define defer2(cap_var1, cap_var2, code) defer2_named(cap_var1, cap_var1, cap_var2, cap_var2, code)


#define defer3_ex(cap_val1, cap_val2, cap_val3, code) \
({ \
    gen_defer_closure_decl(); \
    gen_defer_closure_field_decl(arg0, cap_val1); \
    gen_defer_closure_field_decl(arg1, cap_val2); \
    gen_defer_closure_field_decl(arg2, cap_val3); \
    gen_defer_closure_init()  \
    gen_defer_closure_field_init(arg0, cap_val1); \
    gen_defer_closure_field_init(arg1, cap_val2); \
    gen_defer_closure_field_init(arg2, cap_val3); \
    gen_defer_closure_cb_field_init_part1() \
    gen_defer_closure_cb_field_init_part2(code); \
    gen_defer_end(); \
})

#define defer3_named(closure_var1, cap_val1, closure_var2, cap_val2, closure_var3, cap_val3, code) \
({ \
    gen_defer_closure_decl(); \
    gen_defer_closure_field_decl(closure_var1, cap_val1); \
    gen_defer_closure_field_decl(closure_var2, cap_val2); \
    gen_defer_closure_field_decl(closure_var3, cap_val3); \
    gen_defer_closure_init()  \
    gen_defer_closure_field_init(closure_var1, cap_val1); \
    gen_defer_closure_field_init(closure_var2, cap_val2); \
    gen_defer_closure_field_init(closure_var3, cap_val3); \
    gen_defer_closure_cb_field_init_part1() \
    gen_defer_closure_local_var(closure_var1); \
    gen_defer_closure_local_var(closure_var2); \
    gen_defer_closure_local_var(closure_var3); \
    gen_defer_closure_cb_field_init_part2(code); \
    gen_defer_end(); \
})

#define defer3(cap_var1, cap_var2, cap_var3, code) defer3_named(cap_var1, cap_var1, cap_var2, cap_var2, cap_var3, cap_var3, code)

#define defer4_ex(cap_val1, cap_val2, cap_val3, cap_val4, code) \
({ \
    gen_defer_closure_decl(); \
    gen_defer_closure_field_decl(arg0, cap_val1); \
    gen_defer_closure_field_decl(arg1, cap_val2); \
    gen_defer_closure_field_decl(arg2, cap_val3); \
    gen_defer_closure_field_decl(arg3, cap_val4); \
    gen_defer_closure_init()  \
    gen_defer_closure_field_init(arg0, cap_val1); \
    gen_defer_closure_field_init(arg1, cap_val2); \
    gen_defer_closure_field_init(arg2, cap_val3); \
    gen_defer_closure_field_init(arg3, cap_val4); \
    gen_defer_closure_cb_field_init_part1() \
    gen_defer_closure_cb_field_init_part2(code); \
    gen_defer_end(); \
})

#define defer4_named(closure_var1, cap_val1, closure_var2, cap_val2, closure_var3, cap_val3, closure_var4, cap_val4, code) \
({ \
    gen_defer_closure_decl(); \
    gen_defer_closure_field_decl(closure_var1, cap_val1); \
    gen_defer_closure_field_decl(closure_var2, cap_val2); \
    gen_defer_closure_field_decl(closure_var3, cap_val3); \
    gen_defer_closure_field_decl(closure_var4, cap_val4); \
    gen_defer_closure_init()  \
    gen_defer_closure_field_init(closure_var1, cap_val1); \
    gen_defer_closure_field_init(closure_var2, cap_val2); \
    gen_defer_closure_field_init(closure_var3, cap_val3); \
    gen_defer_closure_field_init(closure_var4, cap_val4); \
    gen_defer_closure_cb_field_init_part1() \
    gen_defer_closure_local_var(closure_var1); \
    gen_defer_closure_local_var(closure_var2); \
    gen_defer_closure_local_var(closure_var3); \
    gen_defer_closure_local_var(closure_var4); \
    gen_defer_closure_cb_field_init_part2(code); \
    gen_defer_end(); \
})

#define defer4(cap_var1, cap_var2, cap_var3, cap_var4, code) defer4_named(cap_var1, cap_var1, cap_var2, cap_var2, cap_var3, cap_var3, cap_var4, cap_var4, code)

#endif
