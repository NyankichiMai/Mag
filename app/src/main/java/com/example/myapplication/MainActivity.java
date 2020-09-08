package com.example.myapplication;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.math.BigDecimal;
import java.util.Date;

import static java.lang.StrictMath.pow;
import static java.lang.StrictMath.sqrt;
import org.apache.http.util.EncodingUtils;

public class MainActivity extends AppCompatActivity implements SensorEventListener {
    private TextView textMagX,textMagY,textMagZ,textTotal,textCount,textApp;
    private SensorManager sensorManager;
    private Button bt;
    boolean save = false;
    //设定窗口大小，步长，app数据类型等
    int window_size = 50;
    int window_step = 20;
    int data_size = 3;
    double [][] data = new double[window_size][data_size];
    private int count = 0;
    private long time1;
    private long time2;
    String filename = "testFile.txt";
    String string = "X Y Z\n";
    String[] flatweights;
    String[] flattest;
    float weights[];
    float tests[];
    String[] PCAMatrixString;
    double[][] PCAMainFeatureMatrix;
    String App = "NULL";
    //String test = "";
    File file;
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    String[] MagdataString;
    double[][] RawMagdata;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        //Intent startIntent = new Intent(this, MyService.class);
        //startService(startIntent);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        bt = (Button) findViewById(R.id.bt1);
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(save == false){
                    save = true;
                    bt.setText("Saving");
                }
                else{
                    save = false;
                    bt.setText("Save");
                }
            }
        });
        textMagX = findViewById(R.id.et1);
        textMagY = findViewById(R.id.et2);
        textMagZ = findViewById(R.id.et3);
        textTotal = findViewById(R.id.et4);
        textCount = findViewById(R.id.et5);
        textApp = findViewById(R.id.et6);
        String result = null;
        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        time1 = new Date().getTime();
        sensorManager.registerListener(this,
                sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD),0);
        //创建文件存储路径，可自行修改
        file = new File(getExternalFilesDir(null).getAbsolutePath()+"/", filename);
        Log.v("Location",file.getAbsolutePath());
        //从raw中读取参数，测试用数据等
        try {
            InputStream in = getResources().openRawResource(R.raw.flatweights);
            int lenght = in.available();
            byte[]  buffer = new byte[lenght];
            in.read(buffer);
            result = EncodingUtils.getAsciiString(buffer);
        } catch (Exception e) {
            e.printStackTrace();
        }
        flatweights = result.split("\n");
        weights = new float[flatweights.length];
        for(int i=0;i<flatweights.length;i++){
            weights[i] = Float.parseFloat(flatweights[i]);
        }
        try {
            InputStream in = getResources().openRawResource(R.raw.flattest);
            int lenght = in.available();
            byte[]  buffer = new byte[lenght];
            in.read(buffer);
            result = EncodingUtils.getAsciiString(buffer);
            InputStream in1 = getResources().openRawResource(R.raw.pca_mainfeature_matrix1);
            int length1 = in1.available();
            byte[] buffer1 = new byte[length1];
            in1.read(buffer1);
            String result1= EncodingUtils.getAsciiString(buffer1);
            PCAMatrixString = result1.split("\n");
            String[] rowstr1 = PCAMatrixString[0].split(" ");
            int pca_feature_num = rowstr1.length;
            PCAMainFeatureMatrix = new double[PCAMatrixString.length][pca_feature_num-1];
            for(int i=0;i<PCAMatrixString.length;i++){
                rowstr1 = PCAMatrixString[i].split(" ");
                for(int j=0;j<pca_feature_num-1;j++){
                    PCAMainFeatureMatrix[i][j] = Double.valueOf(rowstr1[j]);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        flattest = result.split("\n");
        tests = new float[flattest.length];
        for(int i=0;i<flattest.length;i++){
            tests[i] = Float.parseFloat(flattest[i]);
        }
        //传入参数，只需调用stringFromJNI即可，setText仅作测试用，tests参数也是做测试用，可以删去并修改相应的函数
        textApp.setText("APP:"+stringFromJNI(weights,tests));

        //test
        //try {
        //    InputStream in = getResources().openRawResource(R.raw.word);
        //    int length=in.available();
        //    byte[] buffer = new byte[length];
        //    in.read(buffer);
        //    result= EncodingUtils.getAsciiString(buffer);
        //    MagdataString = result.split("\n");
        //    RawMagdata = new double[MagdataString.length-1][3];
        //    for(int i=1;i<MagdataString.length;i++){
        //        String[] rowstr = MagdataString[i].split(" ");
        //        RawMagdata[i-1][0] = Double.valueOf(rowstr[0]);
        //        RawMagdata[i-1][1] = Double.valueOf(rowstr[1]);
        //        RawMagdata[i-1][2] = Double.valueOf(rowstr[2]);
        //    }
        //} catch (Exception e) {
        //    e.printStackTrace();
        //}
        //int i;
        //Log.v("MSG_Start","#################################################");
        //while (count<7000) {
        //    if (count > (window_size - window_step)) {
        //        data[window_size - window_step + ((count- window_size + window_step) % window_step)][0] = RawMagdata[count][0];
        //        data[window_size - window_step + ((count- window_size + window_step) % window_step)][1] = RawMagdata[count][1];
        //        data[window_size - window_step + ((count- window_size + window_step) % window_step)][2] = RawMagdata[count][2];
        //        count++;
        //    } else {
        //        data[count][0] = RawMagdata[count][0];
        //        data[count][1] = RawMagdata[count][1];
        //        data[count][2] = RawMagdata[count][2];
        //        count++;
        //    }
        //    if (count > (window_size - window_step) && (count- (window_size - window_step))% window_step == 0) {
        //        //调用函数
        //        App = DataProcess(data, PCAMainFeatureMatrix);
        //        //for(i=0;i<50;i++)
        //        //    Log.v("MSG",Integer.toString(i)+"  "+Double.toString(data[i][0]));
        //        //Log.v("MSG","########");
        //        Log.v("MSG_APP",App);
        //        Log.v("MSG_Count", String.valueOf(count)+"#######");
        //        for (i = 0; i < (window_size - window_step); i++) {
        //            data[i][0] = data[i + window_step][0];
        //            data[i][1] = data[i + window_step][1];
        //            data[i][2] = data[i + window_step][2];
        //        }
        //    }
        //}
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI(float[] array,float[] array1);
    public native String DataProcess(double[][] magdata,double[][] feature_matrix);

    @Override
    public void onSensorChanged(SensorEvent sensorEvent) {  //检测磁感应信号强度数据
        int i;
        time2 = new Date().getTime();
        float [] values = sensorEvent.values;
        int sensorType = sensorEvent.sensor.getType();
        StringBuilder stringBuilderX = null,stringBuilderY = null,stringBuilderZ = null,stringBuilderT = null;
        if(sensorType == Sensor.TYPE_MAGNETIC_FIELD){
            stringBuilderX = new StringBuilder();
            stringBuilderY = new StringBuilder();
            stringBuilderZ = new StringBuilder();
            stringBuilderT = new StringBuilder();
            stringBuilderX.append(values[0]);
            stringBuilderY.append(values[1]);
            stringBuilderZ.append(values[2]);
            stringBuilderT.append(sqrt(pow(values[0],2)+pow(values[1],2)+pow(values[2],2)));
            textMagX.setText("X:" + stringBuilderX.toString());
            textMagY.setText("Y:" +stringBuilderY.toString());
            textMagZ.setText("Z:"+stringBuilderZ.toString());
            textTotal.setText("Total:"+stringBuilderT.toString());
            textCount.setText("Realtime Sampling rate:"+Float.toString((float)(count*1000/((int)(time2 - time1)))));
            textCount.setText(Integer.toString(count));

            if (count > (window_size - window_step)) {
                data[window_size - window_step + ((count- window_size + window_step) % window_step)][0] = values[0];
                data[window_size - window_step + ((count- window_size + window_step) % window_step)][1] = values[1];
                data[window_size - window_step + ((count- window_size + window_step) % window_step)][2] = values[2];
                count++;
            } else {
                data[count][0] = values[0];
                data[count][1] = values[1];
                data[count][2] = values[2];
                count++;
            }
            if (count > (window_size - window_step) && (count- (window_size - window_step))% window_step == 0) {
                //调用分类函数，传入data数据
                App = DataProcess(data, PCAMainFeatureMatrix);
                for (i = 0; i < 50; i++)
                    Log.v("MSG", Integer.toString(i) + " " + Double.toString(data[i][0]));
                Log.v("MSG", "#####################################");
                for (i = 0; i < (window_size - window_step); i++) {
                    data[i][0] = data[i + window_step][0];
                    data[i][1] = data[i + window_step][1];
                    data[i][2] = data[i + window_step][2];
                }
            }
            textApp.setText(App);
            if(save) {
                string += stringBuilderX.toString() + ' ';
                string += stringBuilderY.toString() + ' ';
                string += stringBuilderZ.toString() + '\n';
                try {
                    FileOutputStream outStream = new FileOutputStream(file);
                    outStream.write(string.getBytes());
                    outStream.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            else {
                string = "X Y Z\n";
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {
    }
}
