#define WIDTH 16
#define HEIGHT 16

unsigned char image_1[16*16*3] = {
    1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
    0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,
    0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,
    0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
    1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
    0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,
    0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,
    0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
    1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
    0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,
    0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,
    0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
    1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
    0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,
    0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,
    0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0
};

unsigned char image_2[16*16*3] = {
    2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,
    0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,
    0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,
    0,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,0,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,
    2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,
    0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,
    0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,
    0,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,0,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,
    2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,
    0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,
    0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,
    0,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,0,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,
    2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,
    0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,
    0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,
    0,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,0,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0
};

unsigned char image_3[16*16*3] = {
    3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,
    0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,
    0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,
    0,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,0,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,
    3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,
    0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,
    0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,
    0,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,0,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,
    3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,
    0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,
    0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,
    0,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,0,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,
    3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,
    0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,
    0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,
    0,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,0,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0,3,0,0
};

unsigned char image_4[16*16*3] = {
    4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,
    0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,
    0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,
    0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,
    4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,
    0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,
    0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,
    0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,
    4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,
    0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,
    0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,
    0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,
    4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,
    0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,
    0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,
    0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4,0,0,4
};