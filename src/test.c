void test_func(void);
#ifdef CONFIG_ARCH_AARCH64
void test_func(void) {
    asm volatile(
        "mov  x1, x0\n\t"
        :
        :
        :);
}
#elif defined(CONFIG_ARCH_RISCV)
void test_func(void) {
    asm volatile(
        "li  t0, 3\n\t"
        :
        :
        :);
}
#endif
