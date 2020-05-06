int my_puts(char *str);

void my_exit();

void fun(){
    char str[] = "Hello World!";

    my_puts(str);
}

int _start(){
    fun();
    my_exit();
}
