int fork();
void cons_putc(int c);
void puts(char* s);
int pipe(int fd[]);
int write(int fd, char* buf, int n);
int read(int fd, char* buf, int n);
void cons_puts(char* s, int size);
