/**
 * c_scope_guard
 * implement defer & scope_guard library for C
 * just keep temp-resource managing easier.
 * by: cloudsong @ 2024
 * License: MIT
 */

#ifndef __simple_c_scopeguard_h__
#define __simple_c_scopeguard_h__

// ==========================[ ScopeExit library]==============================

/// concot two token into one
/// to escape some gcc's marco expanding rule
#define __CONCAT_X(a, ...) a ## __VA_ARGS__

/// @brief closure obj's common head for scope-closure
typedef struct _scope_closure_head {
    void (* callback)(struct _scope_closure_head* self);
} scope_closure_head_t;

static inline void __on_scope_closure_release(void* pobj) {
    scope_closure_head_t* closure = (scope_closure_head_t*)pobj;
    closure->callback(closure);
}

#define gen_scope_closure_decl() \
    __attribute__((cleanup(__on_scope_closure_release))) \
    struct { \
        void (* callback)(void* self)

#define gen_scope_closure_field_decl(var_name, cap_val) \
        typeof(cap_val) var_name

#define gen_scope_closure_cb_field_init_part1(var_id) \
    } __CONCAT_X(__scope_closure , var_id ) = { \
        (void(*)(void*)) ({ \
            void __fn(typeof( __CONCAT_X( __scope_closure , var_id))* __curr_closure) {

#define gen_scope_closure_local_var(var_name) \
                typeof(__curr_closure->var_name) var_name = __curr_closure->var_name

#define gen_scope_closure_cb_field_init_part2(body) \
                body ; \
            }; \
            __fn; \
        })

#define gen_scope_closure_field_init(a_field) \
        a_field

#define gen_scope_closure_end() \
    }

#define scope_arg(arg_pos) __curr_closure->arg ## arg_pos

// -----------------------------------------------------------------------------

/// @brief execute code when exiting the scope
#define scope_exit(code) \
    gen_scope_closure_decl(); \
    gen_scope_closure_cb_field_init_part1(__LINE__) \
    gen_scope_closure_cb_field_init_part2(code) \
    gen_scope_closure_end();

/// @brief capture the value of val1, execute code when exiting the scope
#define scope_exit1_ex(val1, code) \
    gen_scope_closure_decl(); \
    gen_scope_closure_field_decl(arg0, val1); \
    gen_scope_closure_cb_field_init_part1(__LINE__) \
    gen_scope_closure_cb_field_init_part2(code), \
    gen_scope_closure_field_init(val1) \
    gen_scope_closure_end()

#define scope_exit1_named(var1, val1, code) \
    gen_scope_closure_decl(); \
    gen_scope_closure_field_decl(var1, val1); \
    gen_scope_closure_cb_field_init_part1(__LINE__) \
    gen_scope_closure_local_var(var1); \
    gen_scope_closure_cb_field_init_part2(code), \
    gen_scope_closure_field_init(val1) \
    gen_scope_closure_end()

#define scope_exit1(var1, code) \
    scope_exit1_named(var1, var1, code)

/// @brief capture the value of val1-1, execute code when exiting the scope
#define scope_exit2_ex(val1, val2, code) \
    gen_scope_closure_decl(); \
    gen_scope_closure_field_decl(arg0, val1); \
    gen_scope_closure_field_decl(arg1, val2); \
    gen_scope_closure_cb_field_init_part1(__LINE__) \
    gen_scope_closure_cb_field_init_part2(code), \
    gen_scope_closure_field_init(val1), \
    gen_scope_closure_field_init(val2)  \
    gen_scope_closure_end()

#define scope_exit2_named(var1, val1, var2, val2, code) \
    gen_scope_closure_decl(); \
    gen_scope_closure_field_decl(var1, val1); \
    gen_scope_closure_field_decl(var2, val2); \
    gen_scope_closure_cb_field_init_part1(__LINE__) \
    gen_scope_closure_local_var(var1); \
    gen_scope_closure_local_var(var2); \
    gen_scope_closure_cb_field_init_part2(code), \
    gen_scope_closure_field_init(val1), \
    gen_scope_closure_field_init(val2)  \
    gen_scope_closure_end()

#define scope_exit2(var1, var2, code) \
    scope_exit2_named(var1, var1, var2, var2, code)

/// @brief capture the value of val1-3, execute code when exiting the scope
#define scope_exit3_ex(val1, val2, val3, code) \
    gen_scope_closure_decl(); \
    gen_scope_closure_field_decl(arg0, val1); \
    gen_scope_closure_field_decl(arg1, val2); \
    gen_scope_closure_field_decl(arg2, val3); \
    gen_scope_closure_cb_field_init_part1(__LINE__) \
    gen_scope_closure_cb_field_init_part2(code), \
    gen_scope_closure_field_init(val1), \
    gen_scope_closure_field_init(val2), \
    gen_scope_closure_field_init(val3)  \
    gen_scope_closure_end()


#define scope_exit3_named(var1, val1, var2, val2, var3, val3, code) \
    gen_scope_closure_decl(); \
    gen_scope_closure_field_decl(var1, val1); \
    gen_scope_closure_field_decl(var2, val2); \
    gen_scope_closure_field_decl(var3, val3); \
    gen_scope_closure_cb_field_init_part1(__LINE__) \
    gen_scope_closure_local_var(var1); \
    gen_scope_closure_local_var(var2); \
    gen_scope_closure_local_var(var3); \
    gen_scope_closure_cb_field_init_part2(code), \
    gen_scope_closure_field_init(val1), \
    gen_scope_closure_field_init(val2), \
    gen_scope_closure_field_init(val3)  \
    gen_scope_closure_end()

#define scope_exit3(var1, var2, var3, code) \
    scope_exit3_named(var1, var1, var2, var2, var3, var3, code)

/// @brief capture the value of val1-4, execute code when exiting the scope
#define scope_exit4_ex(val1, val2, val3, val4, code) \
    gen_scope_closure_decl(); \
    gen_scope_closure_field_decl(arg0, val1); \
    gen_scope_closure_field_decl(arg1, val2); \
    gen_scope_closure_field_decl(arg2, val3); \
    gen_scope_closure_field_decl(arg3, val4); \
    gen_scope_closure_cb_field_init_part1(__LINE__) \
    gen_scope_closure_cb_field_init_part2(code), \
    gen_scope_closure_field_init(val1), \
    gen_scope_closure_field_init(val2), \
    gen_scope_closure_field_init(val3), \
    gen_scope_closure_field_init(val4)  \
    gen_scope_closure_end()

/// @brief capture the value of var1-4, execute code when exiting the scope
///        create alias var for each captured valX
#define scope_exit4_named(var1, val1, var2, val2, var3, val3, var4, val4, code) \
    gen_scope_closure_decl(); \
    gen_scope_closure_field_decl(var1, val1); \
    gen_scope_closure_field_decl(var2, val2); \
    gen_scope_closure_field_decl(var3, val3); \
    gen_scope_closure_field_decl(var4, val4); \
    gen_scope_closure_cb_field_init_part1(__LINE__) \
    gen_scope_closure_local_var(var1); \
    gen_scope_closure_local_var(var2); \
    gen_scope_closure_local_var(var3); \
    gen_scope_closure_local_var(var4); \
    gen_scope_closure_cb_field_init_part2(code), \
    gen_scope_closure_field_init(val1), \
    gen_scope_closure_field_init(val2), \
    gen_scope_closure_field_init(val3), \
    gen_scope_closure_field_init(val4)  \
    gen_scope_closure_end()

/// @brief capture the value of var1-4, execute code when exiting the scope
#define scope_exit4(var1, var2, var3, var4, code) \
    scope_exit4_named(var1, var1, var2, var2, var3, var3, var4, var4, code)

// =============================================================================

#endif
