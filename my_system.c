#define SYS_EXIT 60
#define SYS_WRITE 1

int my_syscall();

int my_strlen(char *str){
    int r;
    for(r=0;str[r]!='\0';r++)
        ;
    return r;
}

int my_puts(char *str){
    my_syscall(SYS_WRITE, 1, str, my_strlen(str));
}

void my_exit(){
    my_syscall(SYS_EXIT, 0);
}
