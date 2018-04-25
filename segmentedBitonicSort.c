#include <stdio.h>
#include <math.h>

//#define DEBUG

typedef float ElemType;

void printArray(ElemType *arr, int n) {
    int i;
    printf("[%f",arr[0]);
    for (i=1; i < n;i++) {
        printf(", %f",arr[i]);
    }
    printf("]\n");
}

void printBinary(int n)
{
    int i;
    for(i=8; i>=0;i--){
        printf("%d",(n>>i)&1);
    }
}

void segmentedBitonicSort(float* data, int* seg_id, int* seg_start, int n, int m)
{/* data包含需要分段排序的n个float值，
    seg_id给出data中n个元素各自所在的段编号。
    seg_start共有m+1个元素，前m个分别给出0..m-1共m个段的起始位置，seg_start[m]保证等于n。*/
    int seg_no;
    for(seg_no=0; seg_no<m; seg_no++){
        /// 计算出每一个段的起始位置，长度，seg_id对应位置
        float *arr = &data[seg_start[seg_no]];
        int *arr_id = &seg_id[seg_start[seg_no]];
        int length = seg_start[seg_no+1] - seg_start[seg_no];
        /// 对一个单独的段使用双调排序算法
        //BitonicSort(A, length，id);
        {// 这一段用大括号括起来，可以作为一个单独的函数
            /// 先构建bitonic sorting network，利用位运算将network信息存储到seg_id中
            int loop_change_flag;
            int hb0,hbi,hbj,hb_len,hb_middle;
            // 使用一个二进制位记录bitonic merge过程的方向，1为升序，0为降序
            // 最后要按升序排列，故先存入1
            for(hbj=0;hbj<length;hbj++){
                arr_id[hbj] = arr_id[hbj]<<1 | 0x01;
            }
            // 每次除以2，将数组切分，并计算每个BM过程的方向，存入seg_id
            loop_change_flag = 1;
            hb0 = 0;
            while(loop_change_flag){
                loop_change_flag = 0;
                for(hbi=hb0; hbi<length&& arr_id[hbi]==arr_id[hb0]; hbi++);
                hb_len = hbi - hb0;
                hb_middle = hb_len/2;
                if(hb_len>=2){
                    loop_change_flag = 1;
                    for(hbj=hb0; hbj < hb0+hb_middle; hbj++){
                        arr_id[hbj] = arr_id[hbj]<<1 | !(arr_id[hbj]&0x01);
                    }
                    for(hbj=hb0+hb_middle; hbj < hb0+hb_len; hbj++){
                        arr_id[hbj] = arr_id[hbj]<<1 | (arr_id[hbj]&0x01);
                    }
                }
                hb0 = hbi;
                if(hbi==length)
                    hb0 = 0;
            }
            #ifdef DEBUG
            for(hbj=0;hbj<length;hbj++){
                printBinary(arr_id[hbj]);
                printf(" ");
            }
            #endif // DEBUG

            // do merges
            /// 循环处理所有BM过程，方向已由seg_id确定
            int cur_array_offset;
            int direction;
            int step;
            float temp, *cur_array;
            int min_power_of_2;
            for(hb0 = 0; hb_len < length;){
                // 找出一个BM过程
                for(hbi=hb0; hbi<length&& arr_id[hbi]==arr_id[hb0]; hbi++);
                hb_len = hbi - hb0;
                if(hb_len>=2){
                    // 计算这个BM过程需要使用多大的Bp网络
                    min_power_of_2=1;
                    while (!(min_power_of_2>=hb_len)){
                        min_power_of_2 = min_power_of_2<<1;
                    }

                    cur_array = arr+hb0;
                    cur_array_offset = hb0;
                    // 计算排序方向
                    direction = (arr_id[cur_array_offset] & 0x01 );

                    /// 开始一个Bitonic Merge过程
                    //merge_order((arr+block_start), min_power_of_2, hb_len, direction);
                    int i,j,k;
                    #ifdef DEBUG
                    printf("\n\n %cBM%d, offset:%d", direction?'+':'-', hb_len, cur_array_offset);
                    #endif // DEBUG
                    // 逐步缩小比较的步长，完成一次排序
                    for (step = min_power_of_2/2; step>0; step/=2) {
                        #ifdef DEBUG
                        printf("\n STEP %d : ", step);
                        #endif // DEBUG
                        for (i=0; i < min_power_of_2; i+=step*2) {
                            for (j=i,k=0; k < step; j++,k++) {
                                if (cur_array_offset+j+step>=hb0+hb_len)
                                    continue;
                                #ifdef DEBUG
                                printf("check %d(%.1f), %d(%.1f). ", cur_array_offset+j, cur_array[j], cur_array_offset+j+step, cur_array[j+step]);
                                #endif // DEBUG
                                if ( direction == (cur_array[j] > cur_array[j+step])) {
                                    // swap
                                    temp = cur_array[j];
                                    cur_array[j] = cur_array[j+step];
                                    cur_array[j+step] = temp;
                                    #ifdef DEBUG
                                    printf("swap. ");
                                    #endif // DEBUG
                                }
                            }
                        }
                    }
                }
                // 一次排序完成后，删除此次排序的id信息
                for(hbj=hb0; hbj < hb0+hb_len; hbj++){
                    arr_id[hbj] = arr_id[hbj]>>1;
                }
                // 一次排序完成后,开始下一次排序.一层结束后，开始下一层.
                hb0 = hbi;
                if(hbi==length)
                    hb0 = 0;
                #ifdef DEBUG
                printf("\n ## ");
                printArray(arr, length);
                #endif // DEBUG
            }
        }
    }
}

int main()
{
    // test input 1
/*
    ElemType data[16] = {10,20,5,9,3,8,12,14,90,0,60,40,23,35,95,18};
    int seg_id[16] =    {0, 0, 0,0,0,0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0};
    int seg_start[2] = {0,16};
    int m=1;
    int n = 16; // number of numbers to be sorted
*/
    // test input 2
/*
    float data[12]={0.8,sqrt(-1.f),sqrt(-1.f),0.5,0,0,-1,sqrt(-1.f),3453,0,-1,0};
    int seg_id[12]={0,  0,         0,         0,  1,1, 1,1,         1,   1,2,2};
    int n=12;
    int m=3;
    int seg_start[4]={0,4,10,12};
*/
    // sample input

    float data[5]={0.8, 0.2, 0.4, 0.6, 0.5};
    int seg_id[5]={0,   0,   1,   1,   1};
    int seg_start[3]={0,2,5};
    int n=5;
    int m=2;


    // print array before
    printArray(data, n);

    segmentedBitonicSort(data, seg_id, seg_start, n, m);

    // output result
    printArray(data,n);

    return 0;
}
