package com.example.arduinocontrol;

import androidx.appcompat.app.AppCompatActivity;
import androidx.cardview.widget.CardView;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;
import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class uvlight extends AppCompatActivity implements OnClickListener{
    private static final int REQUEST_ENABLE_BT = 1;
    private CardView uvLight;
    //listeners for button presses
    Button on, off,clear;
    //textbox below buttons
    TextView t1,t2,t3;

    BluetoothAdapter mBluetoothAdapter;
    BluetoothDevice mDevice;
    ByteArrayInputStream mSocket;
    ConnectedThread mConnectedThread = null;
    ConnectThread mConnectThread = null;

    private static final String TAG = "Bluetooth";

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {

        //Instantiation
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_uvlight);



        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null) {
            Toast.makeText(getApplicationContext(),"BT not supported!", Toast.LENGTH_SHORT).show();
            return;
        }
        if (!mBluetoothAdapter.isEnabled()) {
            //opens a window to ask user to turn on bluetooth
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, 1);
        }
        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
        if (pairedDevices.size() > 0) {
            for (BluetoothDevice device : pairedDevices) {

                    mDevice = device;

            }
        }

        mConnectThread = new ConnectThread(mDevice);
        mConnectThread.start();


        try {
            setw();
        } catch (Exception e) {
            e.printStackTrace();
        }


    }

    @SuppressLint("ClickableViewAccessibility")
    private void setw() throws IOException {
        //text for bluetooth device details
        t1=(TextView)findViewById(R.id.textView3);
        t2=(TextView)findViewById(R.id.response);

        t2.setMovementMethod(new ScrollingMovementMethod());


            String name = mDevice.getName().toString();

            try {
                t1.setText("Connected to:  " + name);
            } catch (Exception e) {
                e.printStackTrace();
            }

        //links on and off to their element IDs
        on=(Button)findViewById(R.id.uvlighton);
        off=(Button)findViewById(R.id.uvlightoff);
        clear=(Button)findViewById(R.id.clear);

        //clears log text
        clear.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                t2.setText("");
            }
        });


        on.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                //sends 'o' char to Arduino
                // method to turn on uv light
                if(mConnectedThread != null) {
                    //String sig = "o";
                    //mConnectedThread.write(sig.getBytes());
                    int sig = 112;
                    mConnectedThread.write(sig);
                }
            }
        });


        off.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                //method to turn off uv light
                if(mConnectedThread != null) {
                    //String sig1 = "f";
                    //mConnectedThread.write(sig1.getBytes());
                    int sig = 113;
                    mConnectedThread.write(sig);

                }
            }
        });


    }
    private class ConnectThread extends Thread {

        private final BluetoothSocket mmSocket;
        private final BluetoothDevice mmDevice;
        //HC-05 unique uuid
        private final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805f9b34fb");

        public ConnectThread(BluetoothDevice device) {
            BluetoothSocket tmp = null;
            mmDevice = device;
            Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();

                    try {
                        //creates RFCOMM socket
                        tmp = device.createRfcommSocketToServiceRecord(MY_UUID);
                        //shows user they are connected
                        Toast.makeText(getApplicationContext(), "Connected to your Arduino", Toast.LENGTH_SHORT).show();
                        //displays bt info in text field
                    } catch (IOException e) {
                        Toast.makeText(getApplicationContext(), "Unable to connect!", Toast.LENGTH_SHORT).show();
                    }

                    mmSocket = tmp;

        }
        public void run() {
            mBluetoothAdapter.cancelDiscovery();
                //need the if for this edge case

                    try {
                        mmSocket.connect();
                    } catch (IOException connectException) {
                       try {
                        if (mSocket != null) {
                            mSocket.close();
                        }
                        } catch (IOException closeException) {
                            Toast.makeText(getApplicationContext(), "Unable to connect socket!", Toast.LENGTH_SHORT).show();
                        }
                        return;
                    }

                    //new connected thread obj
                    mConnectedThread = new ConnectedThread(mmSocket);
                    mConnectedThread.start();

            }

        public void cancel() {

                    try {
                        Log.e(TAG, "- mmSocket CLOSED! -");
                        mmSocket.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }


        }
    }
    //Thread for bluetooth process
    private class ConnectedThread extends Thread {
        //copies bt socket for threading
        private final BluetoothSocket mmSocket;
        //copies input stream obj for threading
        private final InputStream mmInStream;
        //copies output stream obj for threading
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

                    try {
                        tmpIn = socket.getInputStream();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }


                    try {
                        tmpOut = socket.getOutputStream();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                mmInStream = tmpIn;
                mmOutStream = tmpOut;

        }
        //reads from Arduino
        @Override
        public void run() {
            byte[] buffer = new byte[1024];
            int begin = 0;
            int bytes = 0;

                    while (true) {
                        try {
                            bytes += mmInStream.read(buffer, bytes, buffer.length - bytes);
                            for (int i = begin; i < bytes; i++) {
                                //end char for a given message from Arduino
                                if (buffer[i] == "#".getBytes()[0]) {
                                    mHandler.obtainMessage(1, begin, i, buffer).sendToTarget();
                                    begin = i + 1;
                                    if (i == bytes - 1) {
                                        bytes = 0;
                                        begin = 0;
                                    }
                                }
                            }
                        } catch (IOException e) {
                            break;
                        }
                    }

        }

        //writes to the output stream (in this case the Arduino)
        public void write(/*byte[] bytes*/int num) {

                try {
                    Toast.makeText(getApplicationContext(), "sending bytes", Toast.LENGTH_SHORT).show();
                    mmOutStream.write(num);
                } catch (IOException e) {
                    e.printStackTrace();
                }


        }

        public void cancel() {

                try {
                    Log.e(TAG, "- mmSocket CLOSED! -");
                    mmSocket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }

        }
    }

    @Override
    public void onClick(View v)
    {
        try
        {

        }
        catch (Exception e)
        {
            //displays toast message
            Toast.makeText(getApplicationContext(),e.getMessage(), Toast.LENGTH_SHORT).show();

        }
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.e(TAG, "- ON RESUME -");
    }

    public synchronized void onPause() {
        super.onPause();
        Log.e(TAG, "- ON PAUSE -");
    }


    //Suppresses an annoying handler leak warning
    @SuppressLint("HandlerLeak")
    Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            byte[] writeBuf = (byte[]) msg.obj;
            int begin = (int)msg.arg1;
            int end = (int)msg.arg2;


            switch(msg.what) {
                case 1:
                    String writeMessage = new String(writeBuf);
                    writeMessage = writeMessage.substring(begin, end);

                    //sends message to homepage to turn cardview red
                    //string called error contains value of writeMessage
                    Intent intent = new Intent("errorCode").putExtra("error", writeMessage);
                    LocalBroadcastManager.getInstance(uvlight.this).sendBroadcast(intent);

                    //write message to the scrolling textbox
                    t2.append(writeMessage);
                    break;
            }
        }
    };

}

