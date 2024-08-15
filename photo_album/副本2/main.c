#include "photo.h"
#include "retrieval.h"

int main()
{
    // 初始化硬件或系统相关结构体
    PI *pro_inf = pro_init();
    IMAGE *image_inf = link_init();

    // 检查初始化是否成功
    if (pro_inf == (PI *)-1 || image_inf == (IMAGE *)-1)
    {
        fprintf(stderr, "Initialization failed!\n");
        cleanup(pro_inf, image_inf);
        return -1;
    }

    // 插入图片地址
    if (insert_address(image_inf, "images") == -1)
    {
        fprintf(stderr, "Failed to insert image addresses!\n");
        cleanup(pro_inf, image_inf);
        return -1;
    }

    // 显示加载图片
    display_image(pro_inf, image_inf, "load.bmp");

    // 加载界面进度显示
    for (int i = 0; i <= 100; i++)
    {
        usleep(10000);                 // 休眠 10 毫秒
        loading_interface(pro_inf, i); // 更新加载界面进度
    }
    sleep(1);
    // 进入主界面，处理用户交互
    select_interface(pro_inf, image_inf);

    // 清理资源
    cleanup(pro_inf, image_inf);

    return 0;
}
