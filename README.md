# Defer and Scopeguard for C



## Feature

- scope_exit

execute statements on scope exit.

1. basic usage:

```C
scope_exit({
    printf("scopt exit!!!\n");
});

```

2. capture expr-value into scope-closure

```C
char* str1 = strdup("1234567890");

// capture str1 to scope-closure
scope_exit1(str1, free(str1));

str1 = NULL;

```

- defer

same as golang's `defer`. execute statement before function return, release some resouce here.

1. basic usage:

```C
defer_init(1024, NULL); // init defer on stack

// defer statement
defer({
    // you can access external variable by-ref here !
    printf("defer is called!\n");
});

```

2. capture expr-value into defer-closure

```C

char* str1 = malloc(100); // alloc resource

// capture str1 into anonymouse closure and register it
defer1(str1, {
    // captured str1 is local variable of closure
    free(str1); // release resource
});

str1 = NULL;  // str1 is captured by closure, here 


```


## Example

see test1.c for more.

## Requirement

this implementation use following gcc-features:

- nested function

- statement expressions

- cleanup attributes


## License

MIT
