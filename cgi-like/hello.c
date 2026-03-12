#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("<html><body>\n");
    printf("<h1>Hello from CGI!</h1>\n");

    if (argc > 1) {
        printf("<p>Arguments:</p>\n<ul>\n");
        for (int i = 1; i < argc; i++) {
            printf("<li>%s</li>\n", argv[i]);
        }
        printf("</ul>\n");
    }

    printf("</body></html>\n");
    return 0;
}