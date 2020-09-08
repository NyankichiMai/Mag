#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include<algorithm>
#include <opencv2/core.hpp>
using namespace std;
//test: input data
#define win 10
#define dim 10
#define testnum 2

//layer lstm
#define lstm_win 10
#define lstm_in 10
#define lstm_out 8

//layer dense
#define dense_in 24
#define dense_out 3

//layer conv1d
#define conv_out 16
#define bn_out 16

#define predict_dim 3
#define fft_outsize 26
struct LSTM {
    int windowsize = lstm_win;
    int input_dim = lstm_in;
    int output_dim = lstm_out;
    float weight0[lstm_in][lstm_out * 4];
    float weight1[lstm_out][lstm_out * 4];
    float weight2[lstm_out * 4];
};

struct DENSE {
    int input_dim = dense_in;
    int output_dim = dense_out;
    float weight0[dense_in][dense_out];
    float weight1[dense_out];
};

struct BN {
    int windowsize = lstm_win;
    int input_dim = conv_out;
    float weight0[conv_out];
    float weight1[conv_out];
    float weight2[conv_out];
    float weight3[conv_out];
};

struct CONV{
    int filters;
    int kernal_size;
    int input_num;
    int input_dim;
    float*** weight0;
    float* weight1;
    CONV(int a, int b, int c, int d) {
        int i, j, k;
        filters = a;
        kernal_size = b;
        input_num = c;
        input_dim = d;
        weight0 = new float** [kernal_size];
        for (i = 0; i < kernal_size; i++) {
            weight0[i] = new float* [input_dim];
            for (j = 0; j < input_dim; j++) {
                weight0[i][j] = new float[filters];
                for (k = 0; k < filters; k++) {
                    weight0[i][j][k] = 0;
                }
            }
        }
        weight1 = new float[filters];
        for (i = 0; i < filters; i++)
            weight1[i] = 0;
    }
};


BN bn;
LSTM lstm;
DENSE dense;
CONV conv(16, 8, 10, 10);


int count_vector = 0;
float FeatureVector[10][10];
double FFTVector[10][fft_outsize];
string out = " ";

float sigmoid(float x);

void conv_predict(
        float ioutput[win][conv_out],
        float itest[win][lstm_in],
        CONV conv);

void lstm_predict(
        float ioutput[lstm_out],
        float itest[win][lstm_in],
        float weight0[lstm_in][lstm_out * 4],
        float weight1[lstm_out][lstm_out * 4],
        float weight2[lstm_out * 4]);

void dense_predict(
        float ioutput[dense_out],
        float itest[dense_in],
        float weight0[dense_in][dense_out],
        float weight1[dense_out]);

void bnormalization(
        float ioutput[win][conv_out],
        float itest[win][conv_out],
        float weight0[conv_out],
        float weight1[conv_out],
        float weight2[conv_out],
        float weight3[conv_out],
        float momentum);

void activation(
        float ioutput[win][conv_out],
        float itest[win][conv_out]);

void gap(
        float ioutput[conv_out],
        float itest[win][conv_out]);

void concatenate(
        float ioutput[dense_in],
        float itest1[lstm_out],
        float itest2[conv_out]);

void predict(float test[win][dim], float output[predict_dim]);

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */,
        jfloatArray array,
        jfloatArray array1) {
    float* weights;
    float* tests;
    weights = env->GetFloatArrayElements(array,NULL);
    tests = env->GetFloatArrayElements(array1,NULL);

    int i,j,k,index=0;
    //CONV
    for (i = 0; i < conv.kernal_size; i++) {
        for (j = 0; j < conv.input_dim; j++) {
            for (k = 0; k < conv.filters; k++) {
                conv.weight0[i][j][k] = weights[index++];
            }
        }
    }
    for (j = 0; j < conv.filters; j++)
        conv.weight1[j] = weights[index++];
    //BN
    for (j = 0; j < conv.filters; j++)
        bn.weight0[j] = weights[index++];
    for (j = 0; j < conv.filters; j++)
        bn.weight1[j] = weights[index++];
    for (j = 0; j < conv.filters; j++)
        bn.weight2[j] = weights[index++];
    for (j = 0; j < conv.filters; j++)
        bn.weight3[j] = weights[index++];
    //LSTM
    int lstm_total = lstm_out * 4;
    for (i = 0; i < lstm_in; i++)
        for (j = 0; j < lstm_total; j++) {
            lstm.weight0[i][j] = weights[index++];
        }
    for (i = 0; i < lstm_out; i++)
        for (j = 0; j < lstm_total; j++) {
            lstm.weight1[i][j] = weights[index++];
        }
    for (i = 0; i < lstm_total; i++)
        lstm.weight2[i] = weights[index++];
    //DENSE
    for (i = 0; i < dense_in; i++)
        for (j = 0; j < dense_out; j++) {
            dense.weight0[i][j] = weights[index++];
        }
    for (i = 0; i < dense_out; i++)
        dense.weight1[i] = weights[index++];
    //DATA
    int index_1 = 0;
    float test[testnum][win][dim];
    for (i = 0; i < testnum; i++)
        for (j = 0; j < win; j++)
            for (k = 0; k < dim; k++) {
                test[i][j][k] = tests[index_1++];
            }

    float output_lstm[testnum][lstm_out];

    float output_conv[testnum][win][conv_out];
    float output_bn[testnum][win][conv_out];
    float output_gap[testnum][conv_out];

    float output_con[testnum][dense_in];
    float output_dense[testnum][dense_out];
    for (i = 0; i < testnum; i++) {
        lstm_predict(output_lstm[i], test[i], lstm.weight0, lstm.weight1, lstm.weight2);

        conv_predict(output_conv[i], test[i], conv);

        bnormalization(output_bn[i], output_conv[i], bn.weight0, bn.weight1, bn.weight2, bn.weight3, 0.001);

        activation(output_bn[i], output_bn[i]);

        gap(output_gap[i], output_bn[i]);

        concatenate(output_con[i], output_lstm[i], output_gap[i]);

        dense_predict(output_dense[i], output_con[i], dense.weight0, dense.weight1);

    }
    string out = (to_string(output_dense[0][0])+" "+to_string(output_dense[0][1])+" "+to_string(output_dense[0][2]));
    return env->NewStringUTF(out.c_str());
}

void predict(float test[win][dim], float output[predict_dim]){
    float output_lstm[lstm_out];
    float output_conv[win][conv_out];
    float output_bn[win][conv_out];
    float output_gap[conv_out];
    float output_con[dense_in];
    float output_dense[dense_out];
    lstm_predict(output_lstm, test, lstm.weight0, lstm.weight1, lstm.weight2);
    conv_predict(output_conv, test, conv);
    bnormalization(output_bn, output_conv, bn.weight0, bn.weight1, bn.weight2, bn.weight3, 0.001);
    activation(output_bn, output_bn);
    gap(output_gap, output_bn);
    concatenate(output_con, output_lstm, output_gap);
    dense_predict(output_dense, output_con, dense.weight0, dense.weight1);
    int maxPosition =  max_element(output_dense, output_dense+3) - output_dense;
    out = (to_string(output_dense[0])+" "+to_string(output_dense[1])+" "+to_string(output_dense[2]));
    out = to_string(maxPosition+1);
}

void concatenate(
        float ioutput[dense_in],
        float itest1[lstm_out],
        float itest2[conv_out]) {
    int i, j;
    for (i = 0; i < lstm_out; i++) {
        ioutput[i] = itest1[i];
    }
    for (i = 0; i < conv_out; i++) {
        ioutput[i+lstm_out] = itest2[i];
    }
}

void gap(
        float ioutput[conv_out],
        float itest[win][conv_out]) {
    int i, j;
    for (i = 0; i < conv_out; i++) {
        ioutput[i] = 0;
        for (j = 0; j < win; j++) {
            ioutput[i] += itest[j][i];
        }
        ioutput[i] = ioutput[i] / win;
    }
}

void activation(
        float ioutput[win][conv_out],
        float itest[win][conv_out]) {
    int i, j;
    for (i = 0; i < win; i++) {
        for (j = 0; j < conv_out; j++) {
            if (itest[i][j] < 0)
                ioutput[i][j] = 0;
            else ioutput[i][j] = itest[i][j];
        }
    }
}

void conv_predict(
        float ioutput[win][conv_out],
        float itest[win][lstm_in],
        CONV conv) {
    int i, j, k, num, out_num;
    float** itest_new;
    int index = (int)((conv.kernal_size - 1) / 2);
    int test_total = conv.input_num + conv.kernal_size - 1;

    itest_new = new float*[test_total];
    for (i = 0; i < (test_total); i++) {
        itest_new[i] = new float[conv.input_dim];
        for (j = 0; j < conv.input_dim; j++) {
            if (i >= index && i <= (index + conv.input_num - 1))
                itest_new[i][j] = itest[i - index][j];
            else itest_new[i][j] = 0;
        }
    }
    for (num = 0; num < conv.input_num; num++) {
        for (out_num = 0; out_num < conv.filters; out_num++)
            ioutput[num][out_num] = conv.weight1[out_num];
        for (i = 0; i < conv.kernal_size; i++) {
            for (j = 0; j < conv.filters; j++) {
                for (k = 0; k < conv.input_dim; k++) {
                    ioutput[num][j] += conv.weight0[i][k][j] * itest_new[i+num][k];
                }
            }
        }
    }
}

void bnormalization(
        float ioutput[win][conv_out],
        float itest[win][conv_out],
        float weight0[conv_out],
        float weight1[conv_out],
        float weight2[conv_out],
        float weight3[conv_out],
        float momentum) {
    int i, j;
    for (i = 0; i < win; i++) {
        for (j = 0; j < conv_out; j++)
            ioutput[i][j] = itest[i][j] - weight2[j];
        for (j = 0; j < conv_out; j++)
            ioutput[i][j] = ioutput[i][j] / sqrt(weight3[j]+momentum);
        for (j = 0; j < conv_out; j++)
            ioutput[i][j] = ioutput[i][j] * weight0[j];
        for (j = 0; j < conv_out; j++)
            ioutput[i][j] = ioutput[i][j] + weight1[j];
    }
}

float sigmoid(float x) {
    return 1.0 / (1.0 + exp(-x));
}

void lstm_predict(
        float ioutput[lstm_out],
        float itest[win][lstm_in],
        float weight0[lstm_in][lstm_out * 4],
        float weight1[lstm_out][lstm_out * 4],
        float weight2[lstm_out * 4]) {
    int i, j;

    float c_tm[1][lstm_out], h_tm[1][lstm_out];
    for (j = 0; j < lstm_out; j++) {
        c_tm[0][j] = 0;
        h_tm[0][j] = 0;
    }
    float h_t[1][lstm_out], c_t[1][lstm_out];
    int k, t, p;

    for (i = 0; i < win; i++) {
        float ii[1][lstm_out];
        float ff[1][lstm_out];
        float cc[1][lstm_out];
        float oo[1][lstm_out];
        float s[1][lstm_out * 4];

        for (p = 0; p < lstm_out * 4; p++) s[0][p] = 0;

        for (j = 0; j < lstm_in; j++)
            for (k = 0; k < lstm_out * 4; k++)
                s[0][k] += itest[i][j] * weight0[j][k];


        for (j = 0; j < lstm_out; j++)
            for (k = 0; k < lstm_out * 4; k++)
                s[0][k] += h_tm[0][j] * weight1[j][k];

        for (k = 0; k < lstm_out * 4; k++)
            s[0][k] += weight2[k];

        int offset2 = lstm_out * 2;
        int offset3 = lstm_out * 3;
        for (p = 0; p < lstm_out; p++) {
            ii[0][p] = sigmoid(s[0][p]);
            ff[0][p] = sigmoid(s[0][p + lstm_out]);
            cc[0][p] = tanh(s[0][p + offset2]);
            oo[0][p] = sigmoid(s[0][p + offset3]);
        }

        for (p = 0; p < lstm_out; p++) {
            // c_t = ii * cc + ff * c_tm;
            c_t[0][p] = ii[0][p] * cc[0][p] + ff[0][p] * c_tm[0][p];
            h_t[0][p] = oo[0][p] * tanh(c_t[0][p]);

        }

        for (p = 0; p < lstm_out; p++) {
            c_tm[0][p] = c_t[0][p];
            h_tm[0][p] = h_t[0][p];
        }

    }
    for (p = 0; p < lstm_out; p++) {
        ioutput[p] = h_t[0][p];
    }
}

void dense_predict(
        float ioutput[dense_out],
        float itest[dense_in],
        float weight0[dense_in][dense_out],
        float weight1[dense_out]) {
    int i, j;
    float esum = 0;
    for (i = 0; i < dense_out; i++) {
        ioutput[i] = weight1[i];
        for (j = 0; j < dense_in; j++) {
            ioutput[i] += itest[j] * weight0[j][i];
        }
    }
    out = to_string(ioutput[0])+" "+to_string(ioutput[1])+" "+to_string(ioutput[2]);
    for (i = 0; i < dense_out; i++) {
        ioutput[i] = exp(ioutput[i]);
        esum += ioutput[i];
    }
    for (i = 0; i < dense_out; i++) {
        ioutput[i] = ioutput[i] / esum;
    }
    printf("%f \n", ioutput[0]);
    printf("%f \n", ioutput[1]);
    printf("%f \n", ioutput[2]);
}

void PCAProcess(double signal[10][fft_outsize], cv::Mat FeatureMat){
    double mean[fft_outsize];
    for(int i=0;i<fft_outsize;i++){
        mean[i]=0;
        for(int j=0;j<10;j++){
            mean[i]+=signal[j][i];
        }
        mean[i] = mean[i]/10;
    }
    for(int i=0;i<fft_outsize;i++){
        for(int j=0;j<10;j++){
            signal[j][i] -= mean[i];
        }
    }
    cv::Mat SigMat(10,fft_outsize,CV_64FC1);
    for(int i=0;i<10;i++){
        for(int j=0;j<fft_outsize;j++){
            SigMat.ptr<jdouble>(i)[j] = signal[i][j];
        }
    }
    cv::Mat result_mat = SigMat*FeatureMat;
    for(int i=0;i<10;i++){
        for(int j=0;j<10;j++){
            FeatureVector[i][j] = result_mat.ptr<double>(i)[j];
        }
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_DataProcess(JNIEnv *env,jobject thiz,jobjectArray magdata,jobjectArray feature_matrix){
    //pre data processing
    jsize size = env->GetArrayLength(magdata);
    jdoubleArray thisArray;
    jdouble* X=new jdouble[size];
    jdouble* Y=new jdouble[size];
    jdouble* Z=new jdouble[size];
    jdouble* M=new jdouble[size];
    jdouble* M_normal=new jdouble[size];
    jdouble avgx=0,avgy=0,avgz=0;
    for(int i=0;i<size;i++){
        thisArray=(jdoubleArray)env->GetObjectArrayElement(magdata,i);
        jdouble *thisRow=env->GetDoubleArrayElements(thisArray,NULL);
        X[i]=thisRow[0];
        Y[i]=thisRow[1];
        Z[i]=thisRow[2];
        avgx+=thisRow[0];
        avgy+=thisRow[1];
        avgz+=thisRow[2];
    }
    avgx/=size;
    avgy/=size;
    avgz/=size;
    for(int i=0;i<size;i++){
        X[i]-=avgx;
        Y[i]-=avgy;
        Z[i]-=avgz;
        M[i]=sqrt(X[i]*X[i]+Y[i]*Y[i]+Z[i]*Z[i]);
    }
    jdouble M_max = *(std::max_element(M,M+size));
    jdouble M_min = *(std::min_element(M,M+size));
    for(int i=0;i<size;i++){
        M_normal[i]=(M[i]-M_min)/(M_max-M_min);
    }
    //begin fft processing
    std::vector<double> zeroSeq(size);
    std::vector<double> sig(M_normal,M_normal+size);
    std::vector<cv::Mat> newMat;
    cv::Mat mat1(sig);
    cv::Mat mat2(zeroSeq);
    newMat.push_back(mat1);
    newMat.push_back(mat2);
    cv::Mat complexMat(size,1,CV_64FC2);
    cv::merge(newMat,complexMat);
//    char tmp[1000];
//    sprintf(tmp, "%f, %f", complexMat.at<std::complex<double>>(0,0).real(), complexMat.at<std::complex<double>>(1,0).real());
//    __android_log_write(ANDROID_LOG_DEBUG,"111111111", tmp);
    cv::dft(complexMat,complexMat);
    cv::split(complexMat,newMat);
    std::vector<double> realPart(size);
    newMat[0].copyTo(realPart);
    std::vector<double> imagPart(size);
    newMat[1].copyTo(imagPart);
    int outsize = int(size/2)+1;
    jdouble *fft_results=new jdouble[outsize];
    for(int i=0;i<outsize;i++){
        fft_results[i]=sqrt(realPart[i]*realPart[i]+imagPart[i]*imagPart[i]);
    }
    //begin PCA processing
    jsize rows = env->GetArrayLength(feature_matrix);
    thisArray=(jdoubleArray)env->GetObjectArrayElement(feature_matrix,0);
    jsize cols=env->GetArrayLength(thisArray);
    cv::Mat FeatureMat(rows,cols,CV_64FC1);
    jdouble *prow = NULL;
    for(int i=0;i<rows;i++){
        prow=FeatureMat.ptr<jdouble>(i);
        thisArray=(jdoubleArray)env->GetObjectArrayElement(feature_matrix,i);
        jdouble *thisRow=env->GetDoubleArrayElements(thisArray,NULL);
        for(int j=0;j<cols;j++){
            prow[j]=thisRow[j];
        }
    }
    //std::vector<double> sig(SigData,SigData+N);
//    cv::Mat SigMat(1,rows,CV_64FC1);
//    for(int k=0;k<rows;k++){
//        SigMat.ptr<jdouble>(0)[k] = fft_results[k];
//    }
//    cv::Mat result_mat = SigMat*FeatureMat;
//
//    std::vector<double> result_vector;
//    jdouble *results=new jdouble[cols];
//    result_mat.copyTo(result_vector);
    for(int k=0;k<outsize;k++){
        FFTVector[count_vector][k] = fft_results[k];
    }
    //out = to_string(FFTVector[count_vector][0]) + " "+ to_string(FFTVector[count_vector][1]) +" "+ to_string(FFTVector[count_vector][2]);
    count_vector++;
    float* output[10];
    if(count_vector==10){
        count_vector = 0;
        PCAProcess(FFTVector,FeatureMat);
        //out = "Waiting";
        predict(FeatureVector, reinterpret_cast<float *>(output));
        //out = to_string(FeatureVector[0][0]) + " "+ to_string(FeatureVector[0][1]) +" "+ to_string(FeatureVector[0][2]);
    }
    return  env->NewStringUTF(out.c_str());
}
