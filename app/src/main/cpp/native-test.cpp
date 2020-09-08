//
// Created by jiangyuehu on 2020/8/11.
//

#include <jni.h>
#include <string>
#include <fstream>

using namespace std;

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapplication_MainActivity_stringFromJNITest(
        JNIEnv* env,
        jobject /* this */,
        jfloatArray array) {
    float arr[6];
    int num_data = 10;
    int num_label = 2;
    int num_dim = 6;
    float test_1[num_label][num_data][num_dim];
    int label = 0;
    float min=0, current=0;
    for(int i = 0;i<num_label;i++){
        for(int j=0;j<num_data;j++){
            current = 0;
            for(int k=0;k<num_dim;k++){
                current += sqrt(pow(arr[k] - test_1[i][j][k],2));
            }
            if(current<min) {
                min=current;
                label = i;
            }
        }
    }
    string hello = "false";

    //return env->NewStringUTF(line.c_str());

    //int num = sizeof(array)/sizeof(array[0]);
    return label;
}