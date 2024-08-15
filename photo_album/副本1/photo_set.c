#include "photo.h"
#include "list.h"
#include "retrieval.h"

// 初始化硬件或系统相关结构体
PI *pro_init()
{
    // 分配内存
    PI *pro_inf = (PI *)malloc(sizeof(PI));
    if (pro_inf == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for pro_inf: %s\n", strerror(errno));
        return (PI *)-1;
    }
    memset(pro_inf, 0, sizeof(PI));

    // 打开 LCD 设备
    pro_inf->lcd = open(LCD_DEV_PATH, O_RDWR);
    if (pro_inf->lcd == -1)
    {
        fprintf(stderr, "Failed to open LCD device: %s\n", strerror(errno));
        free(pro_inf);
        return (PI *)-1;
    }

    // 获取 LCD 的屏幕信息
    struct fb_var_screeninfo var_inf;
    if (ioctl(pro_inf->lcd, FBIOGET_VSCREENINFO, &var_inf) == -1)
    {
        fprintf(stderr, "Failed to get LCD screen info: %s\n", strerror(errno));
        close(pro_inf->lcd);
        free(pro_inf);
        return (PI *)-1;
    }
    pro_inf->LCD_W = var_inf.xres;
    pro_inf->LCD_H = var_inf.yres;
    printf("LCD width: %d, height: %d\n", pro_inf->LCD_W, pro_inf->LCD_H);

    // 打开触摸屏设备
    pro_inf->touch_lcd = open(TOUCH_LCD_DEV_PATH, O_RDONLY);
    if (pro_inf->touch_lcd == -1)
    {
        fprintf(stderr, "Failed to open touch LCD device: %s\n", strerror(errno));
        close(pro_inf->lcd);
        free(pro_inf);
        return (PI *)-1;
    }

    // 映射 LCD 显存
    pro_inf->screen_size = pro_inf->LCD_H * pro_inf->LCD_W;
    pro_inf->mmap_init_poi = mmap(NULL, pro_inf->screen_size * LCD_PIXEL_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, pro_inf->lcd, 0);
    if (pro_inf->mmap_init_poi == MAP_FAILED)
    {
        fprintf(stderr, "Failed to map memory: %s\n", strerror(errno));
        close(pro_inf->lcd);
        close(pro_inf->touch_lcd);
        free(pro_inf);
        return (PI *)-1;
    }

    return pro_inf;
}

// 初始化图像链表头节点
IMAGE *link_init()
{
    IMAGE *head_node = (IMAGE *)malloc(sizeof(IMAGE));
    if (head_node == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for head_node: %s\n", strerror(errno));
        return (IMAGE *)-1;
    }
    memset(head_node, 0, sizeof(IMAGE));
    INIT_LIST_HEAD(&head_node->small_headed_data);
    head_node->address = NULL;

    return head_node;
}

// 显示图片
int display_image(PI *pro_inf, IMAGE *image_inf, char *file_path)
{
    int bmp = open(file_path, O_RDONLY);
    if (bmp == -1)
    {
        fprintf(stderr, "Failed to open BMP file: %s\n", strerror(errno));
        return -1;
    }

    // 读取图片宽度和高度
    lseek(bmp, 18, SEEK_SET);
    if (read(bmp, &image_inf->image_w, sizeof(int)) == -1 || read(bmp, &image_inf->image_h, sizeof(int)) == -1)
    {
        fprintf(stderr, "Failed to read image size: %s\n", strerror(errno));
        close(bmp);
        return -1;
    }

    int skip = (image_inf->image_w * BMP_PIXEL_BYTES % 4 != 0) ? 4 - image_inf->image_w * BMP_PIXEL_BYTES % 4 : 0;

    lseek(bmp, 54, SEEK_SET);

    size_t rgb_size = image_inf->image_w * image_inf->image_h * BMP_PIXEL_BYTES + skip * image_inf->image_h;
    char *rgb = (char *)malloc(rgb_size);
    if (rgb == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for RGB data: %s\n", strerror(errno));
        close(bmp);
        return -1;
    }

    if (read(bmp, rgb, rgb_size) == -1)
    {
        fprintf(stderr, "Failed to read image data: %s\n", strerror(errno));
        free(rgb);
        close(bmp);
        return -1;
    }

    // 将图片数据拷贝到 LCD 显存
    for (int y = 0, n = 0; y < pro_inf->LCD_H; y++)
    {
        for (int x = 0; x < pro_inf->LCD_W; x++, n += 3)
        {
            if (n + 2 < image_inf->image_w * image_inf->image_h * BMP_PIXEL_BYTES)
            {
                *(pro_inf->mmap_init_poi + (pro_inf->LCD_W * (pro_inf->LCD_H - 1 - y) + x)) =
                    rgb[n] | rgb[n + 1] << 8 | rgb[n + 2] << 16;
            }
        }
        n += skip;
    }

    free(rgb);
    close(bmp);

    return 0;
}

// 读取触摸屏事件
int touch_screen(PI *pro_inf)
{
    read(pro_inf->touch_lcd, &pro_inf->touch, sizeof(pro_inf->touch));

    if (pro_inf->touch.type == EV_ABS)
    {
        if (pro_inf->touch.code == ABS_X)
        {
            pro_inf->TOUCH_X = pro_inf->touch.value * pro_inf->LCD_W / 1024;
        }
        else if (pro_inf->touch.code == ABS_Y)
        {
            pro_inf->TOUCH_Y = pro_inf->touch.value * pro_inf->LCD_H / 600;
        }
    }

    return 0;
}

// 判断是否为 BMP 文件
int is_bmp_file(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    return (dot && !strcasecmp(dot, ".bmp"));
}

// 插入图像文件地址到链表
int insert_address(IMAGE *image_inf, const char *file_path)
{
    DIR *dp = opendir(file_path);
    if (dp == NULL)
    {
        fprintf(stderr, "Failed to open directory: %s\n", strerror(errno));
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        if (!is_bmp_file(entry->d_name))
        {
            continue;
        }

        IMAGE *new_node = (IMAGE *)malloc(sizeof(IMAGE));
        if (new_node == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for new_node: %s\n", strerror(errno));
            closedir(dp);
            return -1;
        }
        memset(new_node, 0, sizeof(IMAGE));

        new_node->address = (char *)malloc(strlen(file_path) + strlen(entry->d_name) + 2);
        if (new_node->address == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for new_node->address: %s\n", strerror(errno));
            free(new_node);
            closedir(dp);
            return -1;
        }

        sprintf(new_node->address, "%s/%s", file_path, entry->d_name);

        list_add_tail(&new_node->small_headed_data, &image_inf->small_headed_data);
    }

    if (closedir(dp) == -1)
    {
        fprintf(stderr, "Failed to close directory: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

// 遍历链表，生成图像路径列表
char *traverse_linked_list(IMAGE *image_inf)
{
    if (image_inf == NULL)
    {
        printf("Error: Invalid head node...\n");
        return (char *)-1;
    }

    if (list_empty(&image_inf->small_headed_data))
    {
        return (char *)-1;
    }

    // 分配内存
    char *path = (char *)malloc(255 * sizeof(char));
    if (path == NULL)
    {
        perror("Memory allocation failed");
        return (char *)-1;
    }

    path[0] = '\0';

    IMAGE *pos = NULL;
    int count = 0;

    // 遍历链表节点
    list_for_each_entry(pos, &image_inf->small_headed_data, small_headed_data)
    {
        if (count + strlen(pos->address) + 1 >= 255)
        {
            printf("Error: Path list too long\n");
            free(path);
            return (char *)-1;
        }
        strcat(path, pos->address);
        strcat(path, " ");
        count += strlen(pos->address) + 1;
    }

    // 移除最后一个分隔符
    if (count > 0)
    {
        path[count - 1] = '\0';
    }
    return path;
}

// 处理触摸事件
int touch_event_handler(PI *pro_inf, int current_mode)
{
    touch_screen(pro_inf);

    if (pro_inf->touch.type == EV_KEY && pro_inf->touch.code == BTN_TOUCH && pro_inf->touch.value == 0)
    {
        switch (current_mode)
        {
        case MAIN_MODE:
            if (pro_inf->TOUCH_X > 105 && pro_inf->TOUCH_X < 200)
            {
                if (pro_inf->TOUCH_Y > 105 && pro_inf->TOUCH_Y < 155)
                    return IMAGES_MODE;
                if (pro_inf->TOUCH_Y > 220 && pro_inf->TOUCH_Y < 272)
                    pro_inf->first_enter_search = true;
                return CATALOG_SEARCH_MODE;
            }
            if (pro_inf->TOUCH_X > 710 && pro_inf->TOUCH_X < 764 && pro_inf->TOUCH_Y > 410 && pro_inf->TOUCH_Y < 455)
            {
                return EXIT_SYSTEM_MODE;
            }
            break;
        case IMAGES_MODE:
            if (pro_inf->TOUCH_X > 0 && pro_inf->TOUCH_X < 50 && pro_inf->TOUCH_Y < 480)
                return PREVIOUS_IMAGE_MODE;
            if (pro_inf->TOUCH_X > 750 && pro_inf->TOUCH_X < 800 && pro_inf->TOUCH_Y < 480)
                return NEXT_IMAGE_MODE;
            if (pro_inf->TOUCH_X > 270 && pro_inf->TOUCH_X < 500 && pro_inf->TOUCH_Y > 380 && pro_inf->TOUCH_Y < 480)
                return EXIT_MODE;
            break;
        case CATALOG_SEARCH_MODE:
            if (pro_inf->TOUCH_X > 270 && pro_inf->TOUCH_X < 500 && pro_inf->TOUCH_Y > 380 && pro_inf->TOUCH_Y < 480)
            {
                return EXIT_MODE;
            }
            break;
        }
    }

    return current_mode;
}

// 显示图像检索结果
int retrieve_display(PI *pro_inf, IMAGE *image_inf)
{
    char *path_list = traverse_linked_list(image_inf);
    int h = 10;
    int count = 0;
    char len[10];
    if (path_list != (char *)-1)
    {
        char *token = strtok(path_list, " ");
        while (token != NULL)
        {
            usleep(30000);
            Display_Ascii(pro_inf, 10, h += 24, token);
            count++;
            token = strtok(NULL, " ");
        }
        sprintf(len, "%d", count);
    }
    Display_Chinese(pro_inf, 450, 20, "图片共有：");
    Display_Ascii(pro_inf, 560, 20, len);
    Display_Chinese(pro_inf, 585, 20, "张");
    return 0;
}

// 选择显示界面
int select_interface(PI *pro_inf, IMAGE *image_inf)
{
    int mode = MAIN_MODE;
    IMAGE *current_img = list_first_entry(&image_inf->small_headed_data, IMAGE, small_headed_data);

    pro_inf->first_enter_search = true; // 初始化为 true

    while (1)
    {
        switch (mode)
        {
        case MAIN_MODE:
            display_image(pro_inf, image_inf, "main.bmp");
            break;
        case IMAGES_MODE:
            if (current_img->address)
                display_image(pro_inf, current_img, current_img->address);
            break;
        case PREVIOUS_IMAGE_MODE:
            if (current_img->small_headed_data.prev == &image_inf->small_headed_data)
                current_img = list_last_entry(&image_inf->small_headed_data, IMAGE, small_headed_data);
            else
                current_img = list_entry(current_img->small_headed_data.prev, IMAGE, small_headed_data);

            mode = IMAGES_MODE;
            break;
        case NEXT_IMAGE_MODE:
            if (current_img->small_headed_data.next == &image_inf->small_headed_data)
                current_img = list_first_entry(&image_inf->small_headed_data, IMAGE, small_headed_data);
            else
                current_img = list_entry(current_img->small_headed_data.next, IMAGE, small_headed_data);

            mode = IMAGES_MODE;
            break;
        case CATALOG_SEARCH_MODE:
            if (pro_inf->first_enter_search)
            {
                display_image(pro_inf, image_inf, "search.bmp");
                retrieve_display(pro_inf, image_inf);
                pro_inf->first_enter_search = false; // 重置标志位
            }
            break;
        case EXIT_MODE:
            display_image(pro_inf, image_inf, "main.bmp");
            mode = MAIN_MODE;
            break;
        case EXIT_SYSTEM_MODE:
            display_image(pro_inf, image_inf, "exit.bmp");
            exit(0);
        }

        mode = touch_event_handler(pro_inf, mode);
    }

    return 0;
}

// 显示加载进度
int loading_interface(PI *pro_inf, int progress)
{
    if (progress < 0)
        progress = 0;
    if (progress > 100)
        progress = 100;

    for (int y = 400; y < 450; y++)
    {
        for (int x = 100; x < 700; x++)
        {
            if (x < (progress * 8))
            {
                *(pro_inf->mmap_init_poi + 800 * y + x) = 0x000ff000;
            }
            else
            {
                *(pro_inf->mmap_init_poi + 800 * y + x) = 0;
            }
        }
    }
    return 0;
}

// 清理资源
void cleanup(PI *pro_inf, IMAGE *image_inf)
{
    if (pro_inf)
    {
        if (pro_inf->mmap_init_poi != MAP_FAILED)
            munmap(pro_inf->mmap_init_poi, pro_inf->screen_size * LCD_PIXEL_BYTES);
        if (pro_inf->lcd != -1)
            close(pro_inf->lcd);
        if (pro_inf->touch_lcd != -1)
            close(pro_inf->touch_lcd);
        free(pro_inf);
    }

    if (image_inf)
    {
        IMAGE *pos, *n;
        list_for_each_entry_safe(pos, n, &image_inf->small_headed_data, small_headed_data)
        {
            if (pos != image_inf)
            {
                list_del(&pos->small_headed_data);
                free(pos->address);
                free(pos);
            }
        }
        free(image_inf);
    }
}
