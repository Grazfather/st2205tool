typedef struct {
    int fd;
    int width;
    int height;
    int bpp;
    int proto;
    char* buff;
    char* oldpix;
} st2205_handle;

st2205_handle *st2205_open(char *dev);
void st2205_close(st2205_handle *h);
void st2205_send_data(st2205_handle *h,char *pixinfo);
void st2205_backlight(st2205_handle *h, int on);
