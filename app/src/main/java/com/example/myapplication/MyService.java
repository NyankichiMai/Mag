package com.example.myapplication;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.IBinder;

import androidx.annotation.Nullable;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

import static javax.xml.transform.OutputKeys.ENCODING;

public class MyService extends Service implements SensorEventListener {
    private SensorManager sensorManager;
    String filename = "testFile.txt";
    String string = "X Y Z\n";
    String result = "";
    File file = new File(getExternalFilesDir(null).getAbsolutePath()+"/",filename);
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    @Override
    public void onCreate() {
        super.onCreate();
        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        sensorManager.registerListener(this,
                sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD),SensorManager.SENSOR_DELAY_GAME);

    }
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        float [] values = event.values;
        StringBuilder stringBuilderX = null,stringBuilderY = null,stringBuilderZ = null,stringBuilderT = null;
        stringBuilderX = new StringBuilder();
        stringBuilderY = new StringBuilder();
        stringBuilderZ = new StringBuilder();
        stringBuilderX.append(values[0]);
        stringBuilderY.append(values[1]);
        stringBuilderZ.append(values[2]);
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

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }
}