int my_puts(char *str);

void my_exit();

void fun(){
    char str1[] = "Hello World!\n";
    char str2[] = "=== simple linker ===\n";

    my_puts(str1);
    my_puts(str2);
}

int _start(){
    fun();
    my_exit();
}
