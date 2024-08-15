#include "photo.h"
#include "retrieval.h"

int main()
{
    // ��ʼ��Ӳ����ϵͳ��ؽṹ��
    PI *pro_inf = pro_init();
    IMAGE *image_inf = link_init();

    // ����ʼ���Ƿ�ɹ�
    if (pro_inf == (PI *)-1 || image_inf == (IMAGE *)-1)
    {
        fprintf(stderr, "Initialization failed!\n");
        cleanup(pro_inf, image_inf);
        return -1;
    }

    // ����ͼƬ��ַ
    if (insert_address(image_inf, "images") == -1)
    {
        fprintf(stderr, "Failed to insert image addresses!\n");
        cleanup(pro_inf, image_inf);
        return -1;
    }

    // ��ʾ����ͼƬ
    display_image(pro_inf, image_inf, "load.bmp");

    // ���ؽ��������ʾ
    for (int i = 0; i <= 100; i++)
    {
        usleep(10000);                 // ���� 10 ����
        loading_interface(pro_inf, i); // ���¼��ؽ������
    }
    sleep(1);
    // ���������棬�����û�����
    select_interface(pro_inf, image_inf);

    // ������Դ
    cleanup(pro_inf, image_inf);

    return 0;
}
