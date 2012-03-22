#include <view/renderer.c>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("please provide exactly one .off file as input.\n");
        return -1;
    }
    Renderer r = rCreate("Hello");

    Mesh m = mReadOff(0, argv[1]);
    rDisplay(r, m);
/*    
    m = mReadOff(0, argv[1]);
    rDisplay(r, m);
    m = mReadOff(0, argv[1]);
    rDisplay(r, m);
    m = mReadOff(0, argv[1]);
    rDisplay(r, m);
*/
    rWait(r);

    return 0;
}
